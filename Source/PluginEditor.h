#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class StereoToSurroundAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StereoToSurroundAudioProcessorEditor (StereoToSurroundAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~StereoToSurroundAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StereoToSurroundAudioProcessor& audioProcessor;

    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Slider mCrossover;
    juce::Label mCrossoverLabel;
    juce::ToggleButton mFullRange;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fullrangeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoToSurroundAudioProcessorEditor)
};
