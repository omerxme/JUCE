#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct Preset
{
    juce::String name;
    float filterValue;
    float resonance;
};

class PresetManager
{
public:
    PresetManager();
    
    const std::vector<Preset>& getPresets() const { return presets; }
    void loadPreset(int index, juce::AudioProcessorValueTreeState& apvts);
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    juce::String getPresetName(int index) const;
    
private:
    std::vector<Preset> presets;
    void initializePresets();
};
