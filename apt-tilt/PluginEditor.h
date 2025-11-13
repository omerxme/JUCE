#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "PresetManager.h"

class APTTiltEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTTiltEditor(APTTiltProcessor&);
    ~APTTiltEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTTiltProcessor& audioProcessor;

    juce::Slider tiltSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tiltAttachment;
    
    juce::ComboBox presetSelector;
    TiltPresetManager presetManager;
    
    juce::Image logoImage;
    
    void drawScrew(juce::Graphics& g, float x, float y);
    juce::String getTiltString(float tilt);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTTiltEditor)
};
