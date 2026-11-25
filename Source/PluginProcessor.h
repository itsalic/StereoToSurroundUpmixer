#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class StereoToSurroundAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    StereoToSurroundAudioProcessor();
    ~StereoToSurroundAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
private:
    //==============================================================================
    enum class Role { other, frontLeft, frontRight, centre, lfe, surroundLeft, surroundRight };

    // Channel groups that share a volume control.
    enum Group { front, centre, lfe, surround, numGroups, noGroup = -1 };

    struct ChannelState
    {
        Role role = Role::other;
        int silentSamples = 0;
        bool empty = false;
        juce::SmoothedValue<float> fillAmount;
    };

    // 3rd-order Butterworth: a 1st-order section followed by a 2nd-order section with Q = 1.
    struct ButterworthFilter
    {
        void design (bool isHighPass, float frequency, double sampleRate);
        void reset();
        float processSample (float sample) noexcept { return secondOrder.processSample (firstOrder.processSample (sample)); }
        void snapToZero() noexcept;

        juce::dsp::IIR::Filter<float> firstOrder, secondOrder;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static Group getGroup (Role role) noexcept;
    void configureChannelRoles();
    void updateFilterCoefficients (bool force);
    void updateTargets (const juce::AudioBuffer<float>& buffer, int numChannels, bool canUpmix);

    static constexpr int maxChannels = 16;

    juce::AudioProcessorValueTreeState parameters;
    juce::AudioParameterInt* highPassFreqParameter = nullptr;
    juce::AudioParameterBool* highPassParameter = nullptr;
    juce::AudioParameterInt* lowPassFreqParameter = nullptr;
    juce::AudioParameterBool* lowPassParameter = nullptr;
    std::array<juce::AudioParameterInt*, numGroups> gainValueParameters {};
    std::array<juce::AudioParameterBool*, numGroups> gainEnabledParameters {};

    double currentSampleRate = 44100.0;
    int silenceHoldSamples = 0;

    std::array<ChannelState, maxChannels> channelStates;
    int numConfiguredChannels = 0;
    int leftIndex = -1;
    int rightIndex = -1;
    int lfeIndex = -1;

    ButterworthFilter highPassLeft, highPassRight, highPassMono, lowPassMono;
    int designedHighPassFreq = -1;
    int designedLowPassFreq = -1;

    juce::SmoothedValue<float> highPassAmount, lowPassAmount, frontHighPassAmount;
    std::array<juce::SmoothedValue<float>, numGroups> groupGains;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoToSurroundAudioProcessor)
};
