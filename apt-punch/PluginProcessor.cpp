#include "PluginProcessor.h"
#include "PluginEditor.h"

APTPunchProcessor::APTPunchProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedPunch.setCurrentAndTargetValue(0.0f);
}

APTPunchProcessor::~APTPunchProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTPunchProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "punch",
        "Punch",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < -0.01f)
                return "Soft " + juce::String(std::abs(value) * 100.0f, 0) + "%";
            else if (value > 0.01f)
                return "Punch " + juce::String(value * 100.0f, 0) + "%";
            else
                return juce::String("Natural");
        }));

    return layout;
}

void APTPunchProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    smoothedPunch.reset(sampleRate, 0.05);
    envelopeLeft = 0.0f;
    envelopeRight = 0.0f;
}

void APTPunchProcessor::releaseResources() {}

bool APTPunchProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

float APTPunchProcessor::processTransient(float sample, float& envelope, float punch, double sampleRate)
{
    float rectified = std::abs(sample);
    float attackTime = 0.001f;
    float releaseTime = 0.1f;
    float attackCoef = std::exp(-1.0f / (attackTime * sampleRate));
    float releaseCoef = std::exp(-1.0f / (releaseTime * sampleRate));
    
    if (rectified > envelope)
        envelope = attackCoef * envelope + (1.0f - attackCoef) * rectified;
    else
        envelope = releaseCoef * envelope + (1.0f - releaseCoef) * rectified;
    
    float transientGain = 1.0f;
    float sustainGain = 1.0f;
    
    if (punch > 0.0f)
    {
        transientGain = 1.0f + punch * 2.0f;
        sustainGain = 1.0f - punch * 0.3f;
    }
    else if (punch < 0.0f)
    {
        transientGain = 1.0f + punch * 0.5f;
        sustainGain = 1.0f - punch * 0.5f;
    }
    
    float envelopeGain = envelope * transientGain + (1.0f - envelope) * sustainGain;
    return sample * envelopeGain;
}

void APTPunchProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float targetPunch = *apvts.getRawParameterValue("punch");
    smoothedPunch.setTargetValue(targetPunch);
    
    if (std::abs(targetPunch) < 0.01f)
    {
        smoothedPunch.skip(buffer.getNumSamples());
        return;
    }
    
    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = totalNumInputChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float currentPunch = smoothedPunch.getNextValue();
        
        leftData[sample] = processTransient(leftData[sample], envelopeLeft, currentPunch, getSampleRate());
        
        if (rightData != nullptr)
            rightData[sample] = processTransient(rightData[sample], envelopeRight, currentPunch, getSampleRate());
    }
}

juce::AudioProcessorEditor* APTPunchProcessor::createEditor()
{
    return new APTPunchEditor(*this);
}

void APTPunchProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTPunchProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTPunchProcessor();
}
