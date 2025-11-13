#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "PresetManager.h"

class APTDriveEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTDriveEditor(APTDriveProcessor&);
    ~APTDriveEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTDriveProcessor& audioProcessor;

    juce::Slider driveSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    
    juce::ComboBox presetSelector;
    DrivePresetManager presetManager;
    
    juce::Image logoImage;
    
    void drawScrew(juce::Graphics& g, float x, float y);
    juce::String getDriveString(float drive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTDriveEditor)
};
