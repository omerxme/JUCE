#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

class APTFilterEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    APTFilterEditor(APTFilterProcessor&);
    ~APTFilterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    APTFilterProcessor& audioProcessor;

    juce::Slider filterSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAttachment;
    
    void drawMetalTexture(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawScrew(juce::Graphics& g, float x, float y);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTFilterEditor)
};
