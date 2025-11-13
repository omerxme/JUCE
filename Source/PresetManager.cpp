#include "PresetManager.h"

PresetManager::PresetManager()
{
    initializePresets();
}

void PresetManager::initializePresets()
{
    presets.clear();
    
    // Warm Low Pass - Smooth, warm sound
    presets.push_back({"Warm Low Pass", -0.65f, 0.707f});
    
    // Bright High Pass - Clean, bright highs
    presets.push_back({"Bright High Pass", 0.45f, 0.707f});
    
    // Telephone - Classic telephone effect
    presets.push_back({"Telephone", 0.75f, 2.5f});
    
    // Radio - Vintage radio sound
    presets.push_back({"Radio", 0.55f, 1.8f});
    
    // DJ Filter - Aggressive sweep
    presets.push_back({"DJ Filter", -0.85f, 3.5f});
    
    // Bypass
    presets.push_back({"Bypass", 0.0f, 0.707f});
}

void PresetManager::loadPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;
    
    const auto& preset = presets[index];
    
    if (auto* filterParam = apvts.getParameter("filter"))
        filterParam->setValueNotifyingHost(filterParam->convertTo0to1(preset.filterValue));
    
    if (auto* resParam = apvts.getParameter("resonance"))
        resParam->setValueNotifyingHost(resParam->convertTo0to1(preset.resonance));
}

juce::String PresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return "";
    
    return presets[index].name;
}
