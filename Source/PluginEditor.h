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
    juce::Slider resonanceSlider;
    VintageKnobLookAndFeel vintageLookAndFeel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    
    void drawMetalTexture(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawScrew(juce::Graphics& g, float x, float y);
    void drawFrequencyDisplay(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawFilterTypeIndicator(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawCircularProgressRing(juce::Graphics& g, float centerX, float centerY, float radius);
    void drawColorCodedZones(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    juce::String getFrequencyString(float filterValue);
    juce::String getFilterTypeString(float filterValue);
    
    float pulsePhase = 0.0f; // For glow pulsing animation

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APTFilterEditor)
};
