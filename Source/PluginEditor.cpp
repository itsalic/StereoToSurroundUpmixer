#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoToSurroundAudioProcessorEditor::StereoToSurroundAudioProcessorEditor (StereoToSurroundAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    mCrossoverLabel.setText("Crossover Freq", juce::dontSendNotification);
    mCrossoverLabel.setFont(20.0f);
    addAndMakeVisible(mCrossoverLabel);
    mFullRange.setButtonText("Full-Range\nSurround Speakers");
    addAndMakeVisible(mFullRange);
    mCrossover.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mCrossover.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible(mCrossover);
    crossoverAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "crossoverfreq", mCrossover));
    fullrangeAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "fullrange", mFullRange));
    setSize (200, 250);
}

StereoToSurroundAudioProcessorEditor::~StereoToSurroundAudioProcessorEditor()
{
}

//==============================================================================
void StereoToSurroundAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    
}

void StereoToSurroundAudioProcessorEditor::resized()
{
    mCrossover.setBounds(getWidth() / 2 - 75, getHeight() - 175, 150, 150);
    mCrossoverLabel.setBounds(getWidth() / 2 - 70, getHeight() - 195, 200, 30);
    mFullRange.setBounds(getWidth() / 2 - 75, 0, 200, 60);
}
