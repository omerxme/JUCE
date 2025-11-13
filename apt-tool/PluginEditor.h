#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

class APTToolEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTToolEditor(APTToolProcessor&);
    ~APTToolEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTToolProcessor& audioProcessor;

    // Input section
    juce::TextButton phaseLButton, phaseRButton;
    juce::ComboBox stereoModeBox;
    juce::Slider widthSlider;
    juce::TextButton monoButton, bassMonoButton;
    
    // Output section
    juce::Slider gainSlider, balanceSlider;
    juce::TextButton muteButton, dcButton;
    
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseLAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseRAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stereoModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bassMonoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> balanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dcAttachment;
    
    juce::Image logoImage;
    
    void setupButton(juce::TextButton& button, const juce::String& text);
    void drawScrew(juce::Graphics& g, float x, float y);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTToolEditor)
};
