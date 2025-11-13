#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct ChorusPreset
{
    juce::String name;
    float amount;
};

class ChorusPresetManager
{
public:
    ChorusPresetManager();
    
    const std::vector<ChorusPreset>& getPresets() const { return presets; }
    void loadPreset(int index, juce::AudioProcessorValueTreeState& apvts);
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    juce::String getPresetName(int index) const;
    
private:
    std::vector<ChorusPreset> presets;
    void initializePresets();
};
