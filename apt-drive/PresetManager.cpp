#include "PresetManager.h"

DrivePresetManager::DrivePresetManager()
{
    initializePresets();
}

void DrivePresetManager::initializePresets()
{
    presets.clear();
    
    presets.push_back({"Clean Warmth", 0.20f});
    presets.push_back({"Tape Saturation", 0.50f});
    presets.push_back({"Analog Thickness", 0.70f});
    presets.push_back({"Gentle Drive", 0.90f});
    presets.push_back({"Maximum", 1.0f});
    presets.push_back({"Bypass", 0.0f});
}

void DrivePresetManager::loadPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;
    
    const auto& preset = presets[index];
    
    if (auto* param = apvts.getParameter("drive"))
        param->setValueNotifyingHost(param->convertTo0to1(preset.drive));
}

juce::String DrivePresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return "";
    
    return presets[index].name;
}
