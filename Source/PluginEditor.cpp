#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoToSurroundAudioProcessorEditor::StereoToSurroundAudioProcessorEditor (StereoToSurroundAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    mHighPassGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mHighPassGain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    mHighPassGain.setTextValueSuffix("Hz");
    mHighPassLabel.setText("Surround Speaker\nRoll-off", juce::dontSendNotification);
    mHighPassLabel.setFont(15.0f);
    mHighPassLabel.setJustificationType(juce::Justification::centred);
    mHighPassLabel.attachToComponent(&mHighPassGain, false);
    mHighPass.setButtonText("On / Off");
    mHighPass.setClickingTogglesState(true);
    mHighPass.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    mHighPass.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(mHighPassGain);
    addAndMakeVisible(mHighPassLabel);
    addAndMakeVisible(mHighPass);

    mLowPassGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mLowPassGain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    mLowPassGain.setTextValueSuffix("Hz");
    mLowPassLabel.setText("Sub-woofer\nRoll-off", juce::dontSendNotification);
    mLowPassLabel.setFont(15.0f);
    mLowPassLabel.setJustificationType(juce::Justification::centred);
    mLowPassLabel.attachToComponent(&mLowPassGain, false);
    mLowPass.setButtonText("On / Off");
    mLowPass.setClickingTogglesState(true);
    mLowPass.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    mLowPass.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(mLowPassGain);
    addAndMakeVisible(mLowPassLabel);
    addAndMakeVisible(mLowPass);

    mCenterGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mCenterGain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    mCenterGain.setTextValueSuffix("%");
    mCenterLabel.setText("Center Channel\nVolume", juce::dontSendNotification);
    mCenterLabel.setFont(15.0f);
    mCenterLabel.setJustificationType(juce::Justification::centred);
    mCenterLabel.attachToComponent(&mCenterGain, false);
    mCenter.setButtonText("On / Off");
    mCenter.setClickingTogglesState(true);
    mCenter.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    mCenter.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(mCenterGain);
    addAndMakeVisible(mCenterLabel);
    addAndMakeVisible(mCenter);

    mFrontGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mFrontGain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    mFrontGain.setTextValueSuffix("%");
    mFrontLabel.setText("Front Channel\nVolumes", juce::dontSendNotification);
    mFrontLabel.setFont(15.0f);
    mFrontLabel.setJustificationType(juce::Justification::centred);
    mFrontLabel.attachToComponent(&mFrontGain, false);
    mFront.setButtonText("On / Off");
    mFront.setClickingTogglesState(true);
    mFront.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    mFront.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(mFrontGain);
    addAndMakeVisible(mFrontLabel);
    addAndMakeVisible(mFront);

    mRearGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mRearGain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    mRearGain.setTextValueSuffix("%");
    mRearLabel.setText("Rear Channel\nVolumes", juce::dontSendNotification);
    mRearLabel.setFont(15.0f);
    mRearLabel.setJustificationType(juce::Justification::centred);
    mRearLabel.attachToComponent(&mRearGain, false);
    mRear.setButtonText("On / Off");
    mRear.setClickingTogglesState(true);
    mRear.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    mRear.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(mRearGain);
    addAndMakeVisible(mRearLabel);
    addAndMakeVisible(mRear);

    mSubGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mSubGain.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    mSubGain.setTextValueSuffix("%");
    mSubLabel.setText("Sub Channel\nVolume", juce::dontSendNotification);
    mSubLabel.setFont(15.0f);
    mSubLabel.setJustificationType(juce::Justification::centred);
    mSubLabel.attachToComponent(&mSubGain, false);
    mSub.setButtonText("On / Off");
    mSub.setClickingTogglesState(true);
    mSub.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
    mSub.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(mSubGain);
    addAndMakeVisible(mSubLabel);
    addAndMakeVisible(mSub);

    highPassGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, "highPassFreq", mHighPassGain);
    highPassAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "highPass", mHighPass));

    lowPassGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "lowPassFreq", mLowPassGain));
    lowPassAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "lowPass", mLowPass));

    centerGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "centerGainValue", mCenterGain));
    centerAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "centerGain", mCenter));

    frontGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "frontGainValue", mFrontGain));
    frontAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "frontGain", mFront));

    rearGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "rearGainValue", mRearGain));
    rearAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "rearGain", mRear));

    subGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "subGainValue", mSubGain));
    subAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "subGain", mSub));

    setSize (600, 450);
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
    mHighPassGain.setBounds(getWidth() / 2 - 220, 70, 200, 150);
    mHighPass.setBounds(getWidth() / 2 - 158, 10, 75, 25);

    mLowPassGain.setBounds(getWidth() / 2 + 30, 70, 200, 150);
    mLowPass.setBounds(getWidth() / 2 + 92, 10, 75, 25);

    mCenterGain.setBounds(getWidth() / 2 - 325, 290, 200, 150);
    mCenter.setBounds(getWidth() / 2 - 263, 230, 75, 25);

    mFrontGain.setBounds(getWidth() / 2 - 175, 290, 200, 150);
    mFront.setBounds(getWidth() / 2 - 113, 230, 75, 25);

    mRearGain.setBounds(getWidth() / 2 - 25, 290, 200, 150);
    mRear.setBounds(getWidth() / 2 + 36, 230, 75, 25);

    mSubGain.setBounds(getWidth() / 2 + 125, 290, 200, 150);
    mSub.setBounds(getWidth() / 2 + 186, 230, 75, 25);
}
