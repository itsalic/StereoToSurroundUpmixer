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
    struct Control
    {
        juce::Slider slider;
        juce::Label label;
        juce::TextButton toggle;
        // Declared after the components so they are destroyed first.
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> toggleAttachment;
    };

    void setUpControl (Control& control, const juce::String& labelText, const juce::String& suffix,
                       const juce::String& valueParameterId, const juce::String& toggleParameterId);
    static void placeControl (Control& control, int x, int y);

    juce::AudioProcessorValueTreeState& valueTreeState;
    Control highPass, lowPass, center, front, rear, sub;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoToSurroundAudioProcessorEditor)
};
