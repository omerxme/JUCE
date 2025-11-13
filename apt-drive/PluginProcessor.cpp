#include "PluginProcessor.h"
#include "PluginEditor.h"

APTDriveProcessor::APTDriveProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedDrive.setCurrentAndTargetValue(0.0f);
}

APTDriveProcessor::~APTDriveProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTDriveProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "drive",
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < 0.01f)
                return juce::String("Clean");
            else
                return juce::String(static_cast<int>(value * 100.0f)) + "%";
        }));

    return layout;
}

void APTDriveProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    smoothedDrive.reset(sampleRate, 0.05); // 50ms smoothing
}

void APTDriveProcessor::releaseResources() {}

bool APTDriveProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

float APTDriveProcessor::softClip(float sample, float drive)
{
    // Soft clipping with smooth curve
    float x = sample * (1.0f + drive * 3.0f);
    
    if (x > 1.0f)
        return 1.0f - std::exp(-(x - 1.0f));
    else if (x < -1.0f)
        return -1.0f + std::exp((x + 1.0f));
    else
        return x;
}

float APTDriveProcessor::tapeSaturation(float sample, float amount)
{
    // Tape-style saturation with even/odd harmonics
    float x = sample * (1.0f + amount * 2.0f);
    
    // Soft knee compression
    float threshold = 0.5f;
    if (std::abs(x) > threshold)
    {
        float excess = std::abs(x) - threshold;
        float compressed = threshold + excess / (1.0f + excess);
        x = (x > 0.0f) ? compressed : -compressed;
    }
    
    // Add subtle harmonics
    float harmonic = std::tanh(x * 1.5f);
    
    return harmonic;
}

void APTDriveProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float targetDrive = *apvts.getRawParameterValue("drive");
    smoothedDrive.setTargetValue(targetDrive);
    
    // If drive is very low, just pass through
    if (targetDrive < 0.01f)
    {
        smoothedDrive.skip(buffer.getNumSamples());
        return;
    }
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float currentDrive = smoothedDrive.getNextValue();
            
            float input = channelData[sample];
            float output;
            
            // 0-40%: Gentle harmonic thickness
            if (currentDrive <= 0.4f)
            {
                float t = currentDrive / 0.4f;
                output = input + tapeSaturation(input, t * 0.3f) * t * 0.2f;
            }
            // 40-80%: Tape compression + harmonics
            else if (currentDrive <= 0.8f)
            {
                float t = (currentDrive - 0.4f) / 0.4f;
                output = tapeSaturation(input, 0.3f + t * 0.4f);
            }
            // 80-100%: Gentle analog clipping
            else
            {
                float t = (currentDrive - 0.8f) / 0.2f;
                float saturated = tapeSaturation(input, 0.7f);
                output = saturated + softClip(saturated, t * 0.3f) * t * 0.3f;
            }
            
            // Automatic gain compensation
            float compensation = 1.0f / (1.0f + currentDrive * 0.5f);
            channelData[sample] = output * compensation;
        }
    }
}

juce::AudioProcessorEditor* APTDriveProcessor::createEditor()
{
    return new APTDriveEditor(*this);
}

void APTDriveProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTDriveProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTDriveProcessor();
}
