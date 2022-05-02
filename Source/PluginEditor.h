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
    juce::Slider mHighPassGain;
    juce::Label mHighPassLabel;
    juce::TextButton mHighPass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> highPassAttachment;

    juce::Slider mLowPassGain;
    juce::Label mLowPassLabel;
    juce::TextButton mLowPass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lowPassAttachment;

    juce::Slider mCenterGain;
    juce::Label mCenterLabel;
    juce::TextButton mCenter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> centerGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> centerAttachment;

    juce::Slider mFrontGain;
    juce::Label mFrontLabel;
    juce::TextButton mFront;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frontGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> frontAttachment;

    juce::Slider mRearGain;
    juce::Label mRearLabel;
    juce::TextButton mRear;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rearGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> rearAttachment;

    juce::Slider mSubGain;
    juce::Label mSubLabel;
    juce::TextButton mSub;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> subAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoToSurroundAudioProcessorEditor)
};
