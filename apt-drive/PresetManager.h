#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct DrivePreset
{
    juce::String name;
    float drive;
};

class DrivePresetManager
{
public:
    DrivePresetManager();
    
    const std::vector<DrivePreset>& getPresets() const { return presets; }
    void loadPreset(int index, juce::AudioProcessorValueTreeState& apvts);
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    juce::String getPresetName(int index) const;
    
private:
    std::vector<DrivePreset> presets;
    void initializePresets();
};
