#include "PresetManager.h"

PunchPresetManager::PunchPresetManager()
{
    initializePresets();
}

void PunchPresetManager::initializePresets()
{
    presets.clear();
    
    presets.push_back({"Pad-like", -0.80f});
    presets.push_back({"Soft", -0.40f});
    presets.push_back({"Natural", 0.0f});
    presets.push_back({"Punchy", 0.40f});
    presets.push_back({"Maximum Punch", 0.80f});
    presets.push_back({"Bypass", 0.0f});
}

void PunchPresetManager::loadPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;
    
    const auto& preset = presets[index];
    
    if (auto* param = apvts.getParameter("punch"))
        param->setValueNotifyingHost(param->convertTo0to1(preset.punch));
}

juce::String PunchPresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return "";
    
    return presets[index].name;
}
