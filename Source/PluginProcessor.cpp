/*
  ==============================================================================

    The processor code for upmixing a Stereo signal to fill a 5.1 Surround sound signal.
    This Plugin was especially designed for windows PC's with the sound output set to 5.1,
    as windows automatically assigns 6 channels even if a stereo source is playing,
    so 4 channels will be 'empty' but present which mmeans other upmixing plugins/software
    will not work because they see windows outputting a 6 channel signal.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

StereoToSurroundAudioProcessor::StereoToSurroundAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::create5point1())
        .withOutput("Output", juce::AudioChannelSet::create5point1())
    ),
    parameters(*this, nullptr, juce::Identifier("StereoToSurroundUpmixerConfig"),
    {
        std::make_unique<juce::AudioParameterInt>("highPassFreq",
                                                    "Surround-speaker low roll-off",
                                                    0,
                                                    500,
                                                    180),
        std::make_unique<juce::AudioParameterBool>("highPass",
                                                   "Surround-speaker low roll-off",
                                                   true),
        std::make_unique<juce::AudioParameterInt>("lowPassFreq",
                                                    "Sub-Woofer high roll-off",
                                                    0,
                                                    500,
                                                    180),
        std::make_unique<juce::AudioParameterBool>("lowPass",
                                                   "Sub-Woofer high roll-off",
                                                   true),
        std::make_unique<juce::AudioParameterInt>("centerGainValue",
                                                    "Center Volume",
                                                    0,
                                                    400,
                                                    100),
        std::make_unique<juce::AudioParameterBool>("centerGain",
                                                   "center gain",
                                                   true),
        std::make_unique<juce::AudioParameterInt>("frontGainValue",
                                                    "Front-Speaker Volume",
                                                    0,
                                                    400,
                                                    100),
        std::make_unique<juce::AudioParameterBool>("frontGain",
                                                   "front gain",
                                                   true),
        std::make_unique<juce::AudioParameterInt>("rearGainValue",
                                                    "Rear-Speaker Volume",
                                                    0,
                                                    400,
                                                    100),
        std::make_unique<juce::AudioParameterBool>("rearGain",
                                                   "rear gain",
                                                   true),
        std::make_unique<juce::AudioParameterInt>("subGainValue",
                                                    "Sub-Woofer Volume",
                                                    0,
                                                    400,
                                                    100),
        std::make_unique<juce::AudioParameterBool>("subGain",
                                                   "sub gain",
                                                   true)
    })
#endif
{
    highPassFreqParameter = parameters.getRawParameterValue("highPassFreq");
    highPassParameter = parameters.getRawParameterValue("highPass");
    lowPassFreqParameter = parameters.getRawParameterValue("lowPassFreq");
    lowPassParameter = parameters.getRawParameterValue("lowPass");
    centerGainParameter = parameters.getRawParameterValue("centerGain");
    frontGainParameter = parameters.getRawParameterValue("frontGain");
    rearGainParameter = parameters.getRawParameterValue("rearGain");
    subGainParameter = parameters.getRawParameterValue("subGain");
    centerGainValueParameter = parameters.getRawParameterValue("centerGainValue");
    frontGainValueParameter = parameters.getRawParameterValue("frontGainValue");
    rearGainValueParameter = parameters.getRawParameterValue("rearGainValue");
    subGainValueParameter = parameters.getRawParameterValue("subGainValue");
}

StereoToSurroundAudioProcessor::~StereoToSurroundAudioProcessor()
{
}

//==============================================================================
const juce::String StereoToSurroundAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StereoToSurroundAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StereoToSurroundAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StereoToSurroundAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StereoToSurroundAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StereoToSurroundAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StereoToSurroundAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoToSurroundAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StereoToSurroundAudioProcessor::getProgramName (int index)
{
    return {};
}

void StereoToSurroundAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StereoToSurroundAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    /*
        NOTE: A new Filter instance is required for each channel as if the same filter is shared,
        the sound quality will degrade massively (introduces noise and feedback)
    */

    if (!*highPassParameter) {
        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients< float >> highPassFilterCoeffArray =
            juce::dsp::FilterDesign< float >::designIIRHighpassHighOrderButterworthMethod(*highPassFreqParameter, sampleRate, 3);

        *highPassFilterL.get<0>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(0);
        *highPassFilterL.get<1>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(1);
        highPassFilterL.prepare(spec);
        highPassFilterL.reset();

        *highPassFilterR.get<0>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(0);
        *highPassFilterR.get<1>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(1);
        highPassFilterR.prepare(spec);
        highPassFilterR.reset();

        *highPassFilterC.get<0>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(0);
        *highPassFilterC.get<1>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(1);
        highPassFilterC.prepare(spec);
        highPassFilterC.reset();

        *highPassFilterRL.get<0>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(0);
        *highPassFilterRL.get<1>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(1);
        highPassFilterRL.prepare(spec);
        highPassFilterRL.reset();

        *highPassFilterRR.get<0>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(0);
        *highPassFilterRR.get<1>().state = *highPassFilterCoeffArray.getObjectPointerUnchecked(1);
        highPassFilterRR.prepare(spec);
        highPassFilterRR.reset();
    }
    if (!*lowPassParameter) {
        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients< float >> lowPassFilterCoeffArray =
            juce::dsp::FilterDesign< float >::designIIRLowpassHighOrderButterworthMethod(*lowPassFreqParameter, sampleRate, 3);

        *lowPassFilterLFE.get<0>().state = *lowPassFilterCoeffArray.getObjectPointerUnchecked(0);
        *lowPassFilterLFE.get<1>().state = *lowPassFilterCoeffArray.getObjectPointerUnchecked(1);
        lowPassFilterLFE.prepare(spec);
        lowPassFilterLFE.reset();
    }
}

void StereoToSurroundAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StereoToSurroundAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
        && !layouts.getMainInputChannelSet().isDisabled();
}
#endif

void StereoToSurroundAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    juce::dsp::AudioBlock<float> block(buffer);
    int numSamples = buffer.getNumSamples();
    bool upmix = false;
    // Only fills the surround channels if they are present and empty
    if (totalNumInputChannels > 2) {
        if (buffer.getRMSLevel(2, 0, numSamples) < 0.00000001f) {
            buffer.copyFrom(2, 0, buffer.getReadPointer(0), numSamples, 0.5f);
            buffer.addFrom(2, 0, buffer.getReadPointer(1), numSamples, 0.5f);
            upmix = true;
        }
    }
    if (totalNumInputChannels > 3) {
        if (*subGainParameter && buffer.getRMSLevel(3, 0, numSamples) < 0.00000001f) {
            buffer.copyFrom(3, 0, buffer.getReadPointer(0), numSamples, 0.5f);
            buffer.addFrom(3, 0, buffer.getReadPointer(1), numSamples, 0.5f);
            upmix = true;
        }
    }
    if (totalNumInputChannels > 5 && *rearGainParameter) {
        if (buffer.getRMSLevel(4, 0, numSamples) < 0.00000001f) {
            buffer.copyFrom(4, 0, buffer.getReadPointer(0), numSamples);
            upmix = true;
        }
        if (buffer.getRMSLevel(5, 0, numSamples) < 0.00000001f) {
            buffer.copyFrom(5, 0, buffer.getReadPointer(1), numSamples);
            upmix = true;
        }
    }
    if (totalNumInputChannels > 1) {
        if (!*frontGainParameter) {
            buffer.clear(0, 0, numSamples);
            buffer.clear(1, 0, numSamples);
        } else if (frontGainValue == *frontGainValueParameter) {
            buffer.applyGain(0, 0, numSamples, static_cast<float>(*frontGainValueParameter) / 100.0f);
            buffer.applyGain(1, 0, numSamples, static_cast<float>(*frontGainValueParameter) / 100.0f);
        } else {
            buffer.applyGainRamp(0, 0, numSamples, static_cast<float>(frontGainValue) / 100.0f, static_cast<float>(*frontGainValueParameter) / 100.0f);
            buffer.applyGainRamp(1, 0, numSamples, static_cast<float>(frontGainValue) / 100.0f, static_cast<float>(*frontGainValueParameter) / 100.0f);
            frontGainValue = *frontGainValueParameter;
        }
    }
    if (totalNumInputChannels > 2) {
        if (!*centerGainParameter) {
            buffer.clear(2, 0, numSamples);
        } else if (centerGainValue == *centerGainValueParameter) {
            buffer.applyGain(2, 0, numSamples, static_cast<float>(*centerGainValueParameter) / 100.0f);
        } else {
            buffer.applyGainRamp(2, 0, numSamples, static_cast<float>(centerGainValue) / 100.0f, static_cast<float>(*centerGainValueParameter) / 100.0f);
            centerGainValue = *centerGainValueParameter;
        }
    }
    if (totalNumInputChannels > 3) {
        if (!*subGainParameter) {
            buffer.clear(3, 0, numSamples);
        } else if (subGainValue == *subGainValueParameter) {
            buffer.applyGain(3, 0, numSamples, static_cast<float>(*subGainValueParameter) / 100.0f);
        } else {
            buffer.applyGainRamp(3, 0, numSamples, static_cast<float>(subGainValue) / 100.0f, static_cast<float>(*subGainValueParameter) / 100.0f);
            subGainValue = *subGainValueParameter;
        }
    }
    if (totalNumInputChannels > 5) {
        if (!*rearGainParameter) {
            buffer.clear(4, 0, numSamples);
            buffer.clear(5, 0, numSamples);
        } else if (rearGainValue == *rearGainValueParameter) {
            buffer.applyGain(4, 0, numSamples, static_cast<float>(*rearGainValueParameter) / 100.0f);
            buffer.applyGain(5, 0, numSamples, static_cast<float>(*rearGainValueParameter) / 100.0f);
        } else {
            buffer.applyGainRamp(4, 0, numSamples, static_cast<float>(rearGainValue) / 100.0f, static_cast<float>(*rearGainValueParameter) / 100.0f);
            buffer.applyGainRamp(5, 0, numSamples, static_cast<float>(rearGainValue) / 100.0f, static_cast<float>(*rearGainValueParameter) / 100.0f);
            rearGainValue = *rearGainValueParameter;
        }
    }
    // Don't apply channel filtering if the input sound is already composed for surround sound
    if (upmix) {
        if (!*highPassParameter) {
            if (totalNumInputChannels > 0) highPassFilterL.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(0, 1)));
            if (totalNumInputChannels > 1) highPassFilterR.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(1, 1)));
            if (totalNumInputChannels > 2) highPassFilterC.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(2, 1)));
            if (totalNumInputChannels > 4) highPassFilterRL.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(4, 1)));
            if (totalNumInputChannels > 5) highPassFilterRR.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(5, 1)));
        }
        if (!*lowPassParameter) {
            if (totalNumInputChannels > 3) lowPassFilterLFE.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(3, 1)));
        }
    }
}

//==============================================================================
bool StereoToSurroundAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* StereoToSurroundAudioProcessor::createEditor()
{
    return new StereoToSurroundAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void StereoToSurroundAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StereoToSurroundAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoToSurroundAudioProcessor();
}
