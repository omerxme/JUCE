#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct TiltPreset
{
    juce::String name;
    float tilt;
};

class TiltPresetManager
{
public:
    TiltPresetManager();
    
    const std::vector<TiltPreset>& getPresets() const { return presets; }
    void loadPreset(int index, juce::AudioProcessorValueTreeState& apvts);
    int getNumPresets() const { return static_cast<int>(presets.size()); }
    juce::String getPresetName(int index) const;
    
private:
    std::vector<TiltPreset> presets;
    void initializePresets();
};
