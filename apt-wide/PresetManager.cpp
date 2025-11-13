#include "PresetManager.h"

WidePresetManager::WidePresetManager()
{
    initializePresets();
}

void WidePresetManager::initializePresets()
{
    presets.clear();
    
    presets.push_back({"Mono", -1.0f});
    presets.push_back({"Narrow", -0.40f});
    presets.push_back({"Natural", 0.0f});
    presets.push_back({"Wide", 0.40f});
    presets.push_back({"Maximum Wide", 0.80f});
    presets.push_back({"Bypass", 0.0f});
}

void WidePresetManager::loadPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;
    
    const auto& preset = presets[index];
    
    if (auto* param = apvts.getParameter("width"))
        param->setValueNotifyingHost(param->convertTo0to1(preset.width));
}

juce::String WidePresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return "";
    
    return presets[index].name;
}
