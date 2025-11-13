#include "PresetManager.h"

ChorusPresetManager::ChorusPresetManager()
{
    initializePresets();
}

void ChorusPresetManager::initializePresets()
{
    presets.clear();
    
    // Subtle Width - gentle stereo enhancement
    presets.push_back({"Subtle Width", 0.25f});
    
    // Vocal Chorus - clean, not too wet
    presets.push_back({"Vocal Chorus", 0.40f});
    
    // Synth Pad - wider, more depth
    presets.push_back({"Synth Pad", 0.60f});
    
    // Ensemble - classic chorus sound
    presets.push_back({"Ensemble", 0.80f});
    
    // Maximum - full wet
    presets.push_back({"Maximum", 1.0f});
    
    // Bypass
    presets.push_back({"Bypass", 0.0f});
}

void ChorusPresetManager::loadPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;
    
    const auto& preset = presets[index];
    
    if (auto* param = apvts.getParameter("amount"))
        param->setValueNotifyingHost(param->convertTo0to1(preset.amount));
}

juce::String ChorusPresetManager::getPresetName(int index) const
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return "";
    
    return presets[index].name;
}
