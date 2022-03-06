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
    using filterDuplicator = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using filterChain = juce::dsp::ProcessorChain<filterDuplicator, filterDuplicator>;
    filterChain lowPassFilterLFE;
    filterChain highPassFilterL;
    filterChain highPassFilterR;
    filterChain highPassFilterC;
    filterChain highPassFilterRL;
    filterChain highPassFilterRR;
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* crossoverParameter = nullptr;
    std::atomic<float>* fullrangeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoToSurroundAudioProcessor)
};
