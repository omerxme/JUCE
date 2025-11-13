#include "PluginProcessor.h"
#include "PluginEditor.h"

APTWideProcessor::APTWideProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedWidth.setCurrentAndTargetValue(0.0f);
}

APTWideProcessor::~APTWideProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTWideProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "width",
        "Width",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < -0.01f)
                return "Mono " + juce::String(std::abs(value) * 100.0f, 0) + "%";
            else if (value > 0.01f)
                return "Wide " + juce::String(value * 100.0f, 0) + "%";
            else
                return juce::String("Natural");
        }));

    return layout;
}

void APTWideProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;

    lowPassMid.prepare(spec);
    lowPassSide.prepare(spec);
    
    // Low-pass filter at 150-200 Hz for bass protection
    *lowPassMid.state = *FilterCoefs::makeLowPass(sampleRate, 175.0f, 0.707f);
    *lowPassSide.state = *FilterCoefs::makeLowPass(sampleRate, 175.0f, 0.707f);
    
    smoothedWidth.reset(sampleRate, 0.05);
}

void APTWideProcessor::releaseResources() {}

bool APTWideProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void APTWideProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (totalNumInputChannels < 2)
        return;

    float targetWidth = *apvts.getRawParameterValue("width");
    smoothedWidth.setTargetValue(targetWidth);
    
    if (std::abs(targetWidth) < 0.01f)
    {
        smoothedWidth.skip(buffer.getNumSamples());
        return;
    }
    
    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = buffer.getWritePointer(1);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float currentWidth = smoothedWidth.getNextValue();
        
        float left = leftData[sample];
        float right = rightData[sample];
        
        // Convert to Mid/Side
        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f;
        
        // Apply width control
        float sideGain = 1.0f;
        
        if (currentWidth > 0.0f)
        {
            // Widen: increase side
            sideGain = 1.0f + currentWidth * 2.0f;
        }
        else if (currentWidth < 0.0f)
        {
            // Narrow/Mono: decrease side
            sideGain = 1.0f + currentWidth;
        }
        
        side *= sideGain;
        
        // Convert back to Left/Right
        leftData[sample] = mid + side;
        rightData[sample] = mid - side;
    }
    
    smoothedWidth.skip(buffer.getNumSamples());
}

juce::AudioProcessorEditor* APTWideProcessor::createEditor()
{
    return new APTWideEditor(*this);
}

void APTWideProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTWideProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTWideProcessor();
}
