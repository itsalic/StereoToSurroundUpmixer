#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoToSurroundAudioProcessorEditor::StereoToSurroundAudioProcessorEditor (StereoToSurroundAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), valueTreeState(vts)
{
    setUpControl(highPass, "Surround Speaker\nRoll-off", "Hz", "highPassFreq", "highPass");
    setUpControl(lowPass, "Sub-woofer\nRoll-off", "Hz", "lowPassFreq", "lowPass");
    setUpControl(center, "Center Channel\nVolume", "%", "centerGainValue", "centerGain");
    setUpControl(front, "Front Channel\nVolumes", "%", "frontGainValue", "frontGain");
    setUpControl(rear, "Rear Channel\nVolumes", "%", "rearGainValue", "rearGain");
    setUpControl(sub, "Sub Channel\nVolume", "%", "subGainValue", "subGain");

    setSize (600, 450);
}

StereoToSurroundAudioProcessorEditor::~StereoToSurroundAudioProcessorEditor()
{
}

void StereoToSurroundAudioProcessorEditor::setUpControl (Control& control, const juce::String& labelText, const juce::String& suffix,
                                                         const juce::String& valueParameterId, const juce::String& toggleParameterId)
{
    control.slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    control.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    control.slider.setTextValueSuffix(suffix);

    control.label.setText(labelText, juce::dontSendNotification);
    control.label.setFont(15.0f);
    control.label.setJustificationType(juce::Justification::centred);
    control.label.attachToComponent(&control.slider, false);

    control.toggle.setButtonText("On / Off");
    control.toggle.setClickingTogglesState(true);
    control.toggle.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    control.toggle.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(control.slider);
    addAndMakeVisible(control.label);
    addAndMakeVisible(control.toggle);

    control.sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, valueParameterId, control.slider);
    control.toggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(valueTreeState, toggleParameterId, control.toggle);
}

//==============================================================================
void StereoToSurroundAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StereoToSurroundAudioProcessorEditor::placeControl (Control& control, int x, int y)
{
    constexpr int sliderWidth = 200, sliderHeight = 150, toggleWidth = 75, toggleHeight = 25;

    control.slider.setBounds(x, y, sliderWidth, sliderHeight);
    control.toggle.setBounds(x + (sliderWidth - toggleWidth) / 2, y - 60, toggleWidth, toggleHeight);
}

void StereoToSurroundAudioProcessorEditor::resized()
{
    const int centreX = getWidth() / 2;

    placeControl(highPass, centreX - 220, 70);
    placeControl(lowPass, centreX + 30, 70);

    placeControl(center, centreX - 325, 290);
    placeControl(front, centreX - 175, 290);
    placeControl(rear, centreX - 25, 290);
    placeControl(sub, centreX + 125, 290);
}