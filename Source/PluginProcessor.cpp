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
        std::make_unique<juce::AudioParameterFloat>("crossoverfreq",
                                                    "Crossover Frequency",
                                                    20.0f,
                                                    500.0f,
                                                    180.f),
        std::make_unique<juce::AudioParameterBool>("fullrange",
                                                   "Full-Range Surround Speakers",
                                                   false)
    })
#endif
{
    crossoverParameter = parameters.getRawParameterValue("crossoverfreq");
    fullrangeParameter = parameters.getRawParameterValue("fullrange");
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
        NOTE: A new Filter instance is required as if the same filter is shared,
        the sound quality will degrade massively (introduces noise and feedback)
    */

    if (!*fullrangeParameter) {
        juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients< float >> highPassFilterCoeffArray =
            juce::dsp::FilterDesign< float >::designIIRHighpassHighOrderButterworthMethod(*crossoverParameter, sampleRate, 3);

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

    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients< float >> lowPassFilterCoeffArray =
        juce::dsp::FilterDesign< float >::designIIRLowpassHighOrderButterworthMethod(*crossoverParameter, sampleRate, 3);

    *lowPassFilterLFE.get<0>().state = *lowPassFilterCoeffArray.getObjectPointerUnchecked(0);
    *lowPassFilterLFE.get<1>().state = *lowPassFilterCoeffArray.getObjectPointerUnchecked(1);
    lowPassFilterLFE.prepare(spec);
    lowPassFilterLFE.reset();
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
    if (totalNumInputChannels > 2 && buffer.getRMSLevel(2, 0, numSamples) < 0.00000001f) {
        buffer.copyFrom(2, 0, buffer.getReadPointer(0), numSamples, 0.5f);
        buffer.addFrom(2, 0, buffer.getReadPointer(1), numSamples, 0.5f);
        upmix = true;
    }
    if (totalNumInputChannels > 3 && buffer.getRMSLevel(3, 0, numSamples) < 0.00000001f) {
        buffer.copyFrom(3, 0, buffer.getReadPointer(2), numSamples);
        upmix = true;
    }
    if (totalNumInputChannels > 4 && buffer.getRMSLevel(4, 0, numSamples) < 0.00000001f) {
        buffer.copyFrom(4, 0, buffer.getReadPointer(0), numSamples);
        upmix = true;
    }
    if (totalNumInputChannels > 5 && buffer.getRMSLevel(5, 0, numSamples) < 0.00000001f) {
        buffer.copyFrom(5, 0, buffer.getReadPointer(1), numSamples);
        upmix = true;
    }
    // Don't apply channel filtering if the input sound is already composed for surround sound
    if (upmix && totalNumOutputChannels) {
        if (!*fullrangeParameter) {
            highPassFilterL.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(0, 1)));
            highPassFilterR.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(1, 1)));
            highPassFilterC.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(2, 1)));
            highPassFilterRL.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(4, 1)));
            highPassFilterRR.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(5, 1)));
        }
        lowPassFilterLFE.process(juce::dsp::ProcessContextReplacing<float>(block.getSubsetChannelBlock(3, 1)));
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
