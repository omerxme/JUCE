#include "PresetManager.h"

TiltPresetManager::TiltPresetManager()
{
    initializePresets();
}

void TiltPresetManager::initializePresets()
{
    presets.clear();
    
    presets.push_back({"Very Dark", -0.80f});
    presets.push_back({"Warm", -0.40f});
    presets.push_back({"Neutral", 0.0f});
    presets.push_back({"Bright", 0.40f});
    presets.push_back({"Very Bright", 0.80f});
    presets.push_back({"Bypass", 0.0f});
}

void TiltPresetManager::loadPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;
    
    const auto& preset = presets[index];
    
    if (auto* param = apvts.getParameter("tilt"))
        param->setValueNotifyingHost(param->convertTo0to1(preset.tilt));
}

juce::String TiltPresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return "";
    
    return presets[index].name;
}
