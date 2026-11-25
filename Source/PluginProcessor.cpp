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

namespace
{
    // Windows pads unused channels with exact zeros, so a peak level this low means "nothing playing".
    constexpr float silenceThreshold = 1.0e-5f;

    // A channel must stay silent this long before it is treated as empty and filled. This stops a
    // genuine surround channel that briefly goes quiet from being filled (and pumping).
    constexpr double silenceHoldSeconds = 0.5;

    // Crossfade time used for every gain, fill and filter on/off change, to avoid clicks.
    constexpr double rampSeconds = 0.05;

    constexpr float minFilterFrequency = 20.0f;

    template <typename ParameterType>
    ParameterType* getTypedParameter (juce::AudioProcessorValueTreeState& state, const juce::String& id)
    {
        auto* parameter = dynamic_cast<ParameterType*> (state.getParameter (id));
        jassert (parameter != nullptr);
        return parameter;
    }
}

StereoToSurroundAudioProcessor::StereoToSurroundAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::create5point1())
        .withOutput("Output", juce::AudioChannelSet::create5point1())
    ),
#else
    :
#endif
    parameters(*this, nullptr, juce::Identifier("StereoToSurroundUpmixerConfig"), createParameterLayout())
{
    highPassFreqParameter = getTypedParameter<juce::AudioParameterInt>(parameters, "highPassFreq");
    highPassParameter = getTypedParameter<juce::AudioParameterBool>(parameters, "highPass");
    lowPassFreqParameter = getTypedParameter<juce::AudioParameterInt>(parameters, "lowPassFreq");
    lowPassParameter = getTypedParameter<juce::AudioParameterBool>(parameters, "lowPass");

    gainValueParameters[front] = getTypedParameter<juce::AudioParameterInt>(parameters, "frontGainValue");
    gainValueParameters[centre] = getTypedParameter<juce::AudioParameterInt>(parameters, "centerGainValue");
    gainValueParameters[lfe] = getTypedParameter<juce::AudioParameterInt>(parameters, "subGainValue");
    gainValueParameters[surround] = getTypedParameter<juce::AudioParameterInt>(parameters, "rearGainValue");

    gainEnabledParameters[front] = getTypedParameter<juce::AudioParameterBool>(parameters, "frontGain");
    gainEnabledParameters[centre] = getTypedParameter<juce::AudioParameterBool>(parameters, "centerGain");
    gainEnabledParameters[lfe] = getTypedParameter<juce::AudioParameterBool>(parameters, "subGain");
    gainEnabledParameters[surround] = getTypedParameter<juce::AudioParameterBool>(parameters, "rearGain");
}

StereoToSurroundAudioProcessor::~StereoToSurroundAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout StereoToSurroundAudioProcessor::createParameterLayout()
{
    // Parameter IDs must not change, otherwise saved settings will no longer load.
    return {
        std::make_unique<juce::AudioParameterInt>("highPassFreq", "Surround-speaker low roll-off", 0, 500, 180),
        std::make_unique<juce::AudioParameterBool>("highPass", "Surround-speaker low roll-off on", true),
        std::make_unique<juce::AudioParameterInt>("lowPassFreq", "Sub-Woofer high roll-off", 0, 500, 180),
        std::make_unique<juce::AudioParameterBool>("lowPass", "Sub-Woofer high roll-off on", true),
        std::make_unique<juce::AudioParameterInt>("centerGainValue", "Center Volume", 0, 400, 100),
        std::make_unique<juce::AudioParameterBool>("centerGain", "Center on", true),
        std::make_unique<juce::AudioParameterInt>("frontGainValue", "Front-Speaker Volume", 0, 400, 100),
        std::make_unique<juce::AudioParameterBool>("frontGain", "Front-Speakers on", true),
        std::make_unique<juce::AudioParameterInt>("rearGainValue", "Rear-Speaker Volume", 0, 400, 100),
        std::make_unique<juce::AudioParameterBool>("rearGain", "Rear-Speakers on", true),
        std::make_unique<juce::AudioParameterInt>("subGainValue", "Sub-Woofer Volume", 0, 400, 100),
        std::make_unique<juce::AudioParameterBool>("subGain", "Sub-Woofer on", true)
    };
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
    // Covers the IIR filter decay and the fill/gain crossfades.
    return 0.1;
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
void StereoToSurroundAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    silenceHoldSamples = juce::roundToInt(sampleRate * silenceHoldSeconds);

    configureChannelRoles();

    updateFilterCoefficients(true);
    for (auto* filter : { &highPassLeft, &highPassRight, &highPassMono, &lowPassMono })
        filter->reset();

    auto resetSmoothed = [sampleRate](juce::SmoothedValue<float>& value, float initialValue)
    {
        value.reset(sampleRate, rampSeconds);
        value.setCurrentAndTargetValue(initialValue);
    };

    resetSmoothed(highPassAmount, highPassParameter->get() ? 1.0f : 0.0f);
    resetSmoothed(lowPassAmount, lowPassParameter->get() ? 1.0f : 0.0f);
    resetSmoothed(frontHighPassAmount, 0.0f);

    for (int group = 0; group < numGroups; ++group)
        resetSmoothed(groupGains[(size_t) group],
                      gainEnabledParameters[(size_t) group]->get() ? (float) gainValueParameters[(size_t) group]->get() / 100.0f : 0.0f);

    // Start out assuming the channels are empty (with the fill faded out); the first block that
    // carries real content switches them back to pass-through before anything is mixed in.
    for (auto& state : channelStates)
    {
        state.silentSamples = silenceHoldSamples;
        state.empty = true;
        resetSmoothed(state.fillAmount, 0.0f);
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

void StereoToSurroundAudioProcessor::configureChannelRoles()
{
    auto layout = getChannelLayoutOfBus(true, 0);

    // Hosts that only report a channel count (e.g. plain VST2) give a discrete layout,
    // so fall back to the standard speaker order for that many channels.
    if (layout.isDiscreteLayout())
        layout = juce::AudioChannelSet::canonicalChannelSet(layout.size());

    numConfiguredChannels = juce::jmin(layout.size(), maxChannels);
    leftIndex = rightIndex = lfeIndex = -1;

    for (int channel = 0; channel < numConfiguredChannels; ++channel)
    {
        auto& role = channelStates[(size_t) channel].role;

        switch (layout.getTypeOfChannel(channel))
        {
            case juce::AudioChannelSet::left:               role = Role::frontLeft;     leftIndex = channel;  break;
            case juce::AudioChannelSet::right:              role = Role::frontRight;    rightIndex = channel; break;
            case juce::AudioChannelSet::centre:             role = Role::centre;        break;
            case juce::AudioChannelSet::LFE:                role = Role::lfe;           lfeIndex = channel;   break;
            case juce::AudioChannelSet::leftSurround:
            case juce::AudioChannelSet::leftSurroundSide:
            case juce::AudioChannelSet::leftSurroundRear:   role = Role::surroundLeft;  break;
            case juce::AudioChannelSet::rightSurround:
            case juce::AudioChannelSet::rightSurroundSide:
            case juce::AudioChannelSet::rightSurroundRear:  role = Role::surroundRight; break;
            default:                                        role = Role::other;         break;
        }
    }
}

StereoToSurroundAudioProcessor::Group StereoToSurroundAudioProcessor::getGroup (Role role) noexcept
{
    switch (role)
    {
        case Role::frontLeft:
        case Role::frontRight:      return front;
        case Role::centre:          return centre;
        case Role::lfe:             return lfe;
        case Role::surroundLeft:
        case Role::surroundRight:   return surround;
        case Role::other:
        default:                    return noGroup;
    }
}

void StereoToSurroundAudioProcessor::ButterworthFilter::design (bool isHighPass, float frequency, double sampleRate)
{
    using Coefficients = juce::dsp::IIR::ArrayCoefficients<float>;

    // A 0 Hz cutoff is degenerate (unstable high-pass, NaN low-pass), and the cutoff must stay below Nyquist.
    frequency = juce::jlimit(minFilterFrequency, (float) (sampleRate * 0.45), frequency);

    // Assigning std::array coefficients reuses the existing storage, so this is safe on the audio thread.
    if (isHighPass)
    {
        *firstOrder.coefficients = Coefficients::makeFirstOrderHighPass(sampleRate, frequency);
        *secondOrder.coefficients = Coefficients::makeHighPass(sampleRate, frequency, 1.0f);
    }
    else
    {
        *firstOrder.coefficients = Coefficients::makeFirstOrderLowPass(sampleRate, frequency);
        *secondOrder.coefficients = Coefficients::makeLowPass(sampleRate, frequency, 1.0f);
    }
}

void StereoToSurroundAudioProcessor::ButterworthFilter::reset()
{
    firstOrder.reset();
    secondOrder.reset();
}

void StereoToSurroundAudioProcessor::ButterworthFilter::snapToZero() noexcept
{
    firstOrder.snapToZero();
    secondOrder.snapToZero();
}

void StereoToSurroundAudioProcessor::updateFilterCoefficients (bool force)
{
    const auto highPassFreq = highPassFreqParameter->get();
    if (force || highPassFreq != designedHighPassFreq)
    {
        for (auto* filter : { &highPassLeft, &highPassRight, &highPassMono })
            filter->design(true, (float) highPassFreq, currentSampleRate);
        designedHighPassFreq = highPassFreq;
    }

    const auto lowPassFreq = lowPassFreqParameter->get();
    if (force || lowPassFreq != designedLowPassFreq)
    {
        lowPassMono.design(false, (float) lowPassFreq, currentSampleRate);
        designedLowPassFreq = lowPassFreq;
    }
}

void StereoToSurroundAudioProcessor::updateTargets (const juce::AudioBuffer<float>& buffer, int numChannels, bool canUpmix)
{
    const int numSamples = buffer.getNumSamples();

    for (int group = 0; group < numGroups; ++group)
        groupGains[(size_t) group].setTargetValue(gainEnabledParameters[(size_t) group]->get()
                                                      ? (float) gainValueParameters[(size_t) group]->get() / 100.0f
                                                      : 0.0f);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto& state = channelStates[(size_t) channel];
        const auto group = getGroup(state.role);

        if (group == noGroup || group == front)
            continue;

        // Only leave the empty state immediately; entering it requires sustained silence.
        if (buffer.getMagnitude(channel, 0, numSamples) <= silenceThreshold)
        {
            state.silentSamples = juce::jmin(state.silentSamples + numSamples, silenceHoldSamples);
            state.empty = state.silentSamples >= silenceHoldSamples;
        }
        else
        {
            state.silentSamples = 0;
            state.empty = false;
        }

        const bool fill = canUpmix && state.empty && gainEnabledParameters[(size_t) group]->get();
        state.fillAmount.setTargetValue(fill ? 1.0f : 0.0f);
    }

    highPassAmount.setTargetValue(highPassParameter->get() ? 1.0f : 0.0f);
    lowPassAmount.setTargetValue(lowPassParameter->get() ? 1.0f : 0.0f);

    // The fronts only lose their lows while the sub is being filled to take them over.
    const bool subIsFilled = lfeIndex >= 0 && lfeIndex < numChannels
                          && channelStates[(size_t) lfeIndex].fillAmount.getTargetValue() > 0.0f;
    frontHighPassAmount.setTargetValue(highPassParameter->get() && subIsFilled ? 1.0f : 0.0f);
}

void StereoToSurroundAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin(buffer.getNumChannels(), numConfiguredChannels);

    if (numSamples == 0 || numChannels == 0)
        return;

    updateFilterCoefficients(false);

    const bool canUpmix = leftIndex >= 0 && leftIndex < numChannels && rightIndex >= 0 && rightIndex < numChannels;
    updateTargets(buffer, numChannels, canUpmix);

    std::array<float*, maxChannels> channelData {};
    for (int channel = 0; channel < numChannels; ++channel)
        channelData[(size_t) channel] = buffer.getWritePointer(channel);

    // The fill signals are always generated (and their filters always run) so that filter state is
    // continuous; they are only faded into channels that are currently empty.
    for (int i = 0; i < numSamples; ++i)
    {
        float filteredLeft = 0.0f, filteredRight = 0.0f;
        float surroundFillLeft = 0.0f, surroundFillRight = 0.0f, centreFill = 0.0f, lfeFill = 0.0f;

        const float highPass = highPassAmount.getNextValue();
        const float lowPass = lowPassAmount.getNextValue();
        const float frontHighPass = frontHighPassAmount.getNextValue();

        if (canUpmix)
        {
            const float left = channelData[(size_t) leftIndex][i];
            const float right = channelData[(size_t) rightIndex][i];
            const float mono = 0.5f * (left + right);

            filteredLeft = highPassLeft.processSample(left);
            filteredRight = highPassRight.processSample(right);
            const float filteredMono = highPassMono.processSample(mono);
            const float lowPassedMono = lowPassMono.processSample(mono);

            surroundFillLeft = left + highPass * (filteredLeft - left);
            surroundFillRight = right + highPass * (filteredRight - right);
            centreFill = mono + highPass * (filteredMono - mono);
            lfeFill = mono + lowPass * (lowPassedMono - mono);
        }

        std::array<float, numGroups> gains;
        for (int group = 0; group < numGroups; ++group)
            gains[(size_t) group] = groupGains[(size_t) group].getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto& state = channelStates[(size_t) channel];
            float& sample = channelData[(size_t) channel][i];

            switch (state.role)
            {
                case Role::frontLeft:     sample = (sample + frontHighPass * (filteredLeft - sample)) * gains[front]; break;
                case Role::frontRight:    sample = (sample + frontHighPass * (filteredRight - sample)) * gains[front]; break;
                case Role::centre:        sample = (sample + state.fillAmount.getNextValue() * centreFill) * gains[centre]; break;
                case Role::lfe:           sample = (sample + state.fillAmount.getNextValue() * lfeFill) * gains[lfe]; break;
                case Role::surroundLeft:  sample = (sample + state.fillAmount.getNextValue() * surroundFillLeft) * gains[surround]; break;
                case Role::surroundRight: sample = (sample + state.fillAmount.getNextValue() * surroundFillRight) * gains[surround]; break;
                case Role::other:
                default:                  break;
            }
        }
    }

    for (auto* filter : { &highPassLeft, &highPassRight, &highPassMono, &lowPassMono })
        filter->snapToZero();
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
