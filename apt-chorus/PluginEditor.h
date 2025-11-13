#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "PresetManager.h"

class APTChorusEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTChorusEditor(APTChorusProcessor&);
    ~APTChorusEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTChorusProcessor& audioProcessor;

    juce::Slider amountSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    
    juce::ComboBox presetSelector;
    ChorusPresetManager presetManager;
    
    juce::Image logoImage;
    
    void drawScrew(juce::Graphics& g, float x, float y);
    juce::String getAmountString(float amount);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTChorusEditor)
};
