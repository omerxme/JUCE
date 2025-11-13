#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "PresetManager.h"

class APTPunchEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTPunchEditor(APTPunchProcessor&);
    ~APTPunchEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTPunchProcessor& audioProcessor;

    juce::Slider punchSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> punchAttachment;
    
    juce::ComboBox presetSelector;
    PunchPresetManager presetManager;
    
    juce::Image logoImage;
    
    void drawScrew(juce::Graphics& g, float x, float y);
    juce::String getPunchString(float punch);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTPunchEditor)
};
