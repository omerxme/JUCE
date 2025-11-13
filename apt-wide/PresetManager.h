#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct WidePreset
{
    juce::String name;
    float width;
};

class WidePresetManager
{
public:
    WidePresetManager();
    
    const std::vector<WidePreset>& getPresets() const { return presets; }
    void loadPreset(int index, juce::AudioProcessorValueTreeState& apvts);
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    juce::String getPresetName(int index) const;
    
private:
    std::vector<WidePreset> presets;
    void initializePresets();
};
