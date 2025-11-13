#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct PunchPreset
{
    juce::String name;
    float punch;
};

class PunchPresetManager
{
public:
    PunchPresetManager();
    
    const std::vector<PunchPreset>& getPresets() const { return presets; }
    void loadPreset(int index, juce::AudioProcessorValueTreeState& apvts);
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    juce::String getPresetName(int index) const;
    
private:
    std::vector<PunchPreset> presets;
    void initializePresets();
};
