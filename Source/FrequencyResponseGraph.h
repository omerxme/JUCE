#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

class FrequencyResponseGraph : public juce::Component
{
public:
    FrequencyResponseGraph();
    
    void paint(juce::Graphics& g) override;
    void updateResponse(float filterValue, float resonance, double sampleRate);
    
    // Make graph non-interactive so it doesn't block knob
    bool hitTest(int x, int y) override { return false; }
    
private:
    std::vector<float> magnitudes;
    float currentFilterValue = 0.0f;
    float currentResonance = 0.707f;
    double currentSampleRate = 44100.0;
    
    void calculateMagnitudes();
    float getMagnitudeForFrequency(float frequency);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponseGraph)
};
