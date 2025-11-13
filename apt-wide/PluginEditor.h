#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "PresetManager.h"

class APTWideEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTWideEditor(APTWideProcessor&);
    ~APTWideEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTWideProcessor& audioProcessor;

    juce::Slider widthSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    
    juce::ComboBox presetSelector;
    WidePresetManager presetManager;
    
    juce::Image logoImage;
    
    void drawScrew(juce::Graphics& g, float x, float y);
    juce::String getWidthString(float width);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTWideEditor)
};
