#include "PluginProcessor.h"
#include "PluginEditor.h"

APTTiltProcessor::APTTiltProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedTilt.setCurrentAndTargetValue(0.0f);
}

APTTiltProcessor::~APTTiltProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTTiltProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tilt",
        "Tilt",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < -0.01f)
                return "Dark " + juce::String(std::abs(value) * 100.0f, 0) + "%";
            else if (value > 0.01f)
                return "Bright " + juce::String(value * 100.0f, 0) + "%";
            else
                return juce::String("Neutral");
        }));

    return layout;
}

void APTTiltProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;

    lowShelfLeft.prepare(spec);
    lowShelfRight.prepare(spec);
    highShelfLeft.prepare(spec);
    highShelfRight.prepare(spec);
    
    smoothedTilt.reset(sampleRate, 0.05);
    
    updateFilters(0.0f, sampleRate);
}

void APTTiltProcessor::releaseResources() {}

bool APTTiltProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void APTTiltProcessor::updateFilters(float tilt, double sampleRate)
{
    const float pivotFreq = 1000.0f;
    const float lowFreq = 200.0f;
    const float highFreq = 5000.0f;
    
    // Calculate gain based on tilt amount (1-3 dB/oct steepness)
    float maxGain = 12.0f; // Maximum boost/cut in dB
    float lowGain = -tilt * maxGain;
    float highGain = tilt * maxGain;
    
    // Low shelf
    *lowShelfLeft.state = *FilterCoefs::makeLowShelf(sampleRate, lowFreq, 0.707f, 
                                                      juce::Decibels::decibelsToGain(lowGain));
    *lowShelfRight.state = *FilterCoefs::makeLowShelf(sampleRate, lowFreq, 0.707f, 
                                                       juce::Decibels::decibelsToGain(lowGain));
    
    // High shelf
    *highShelfLeft.state = *FilterCoefs::makeHighShelf(sampleRate, highFreq, 0.707f, 
                                                        juce::Decibels::decibelsToGain(highGain));
    *highShelfRight.state = *FilterCoefs::makeHighShelf(sampleRate, highFreq, 0.707f, 
                                                         juce::Decibels::decibelsToGain(highGain));
}

void APTTiltProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float targetTilt = *apvts.getRawParameterValue("tilt");
    smoothedTilt.setTargetValue(targetTilt);
    
    // If tilt is very low, just pass through
    if (std::abs(targetTilt) < 0.01f)
    {
        smoothedTilt.skip(buffer.getNumSamples());
        return;
    }
    
    // Update filters
    float currentTilt = smoothedTilt.getCurrentValue();
    updateFilters(currentTilt, getSampleRate());
    
    // Process audio
    juce::dsp::AudioBlock<float> block(buffer);
    
    if (totalNumInputChannels > 0)
    {
        auto leftBlock = block.getSingleChannelBlock(0);
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        lowShelfLeft.process(leftContext);
        highShelfLeft.process(leftContext);
    }
    
    if (totalNumInputChannels > 1)
    {
        auto rightBlock = block.getSingleChannelBlock(1);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        lowShelfRight.process(rightContext);
        highShelfRight.process(rightContext);
    }
    
    smoothedTilt.skip(buffer.getNumSamples());
}

juce::AudioProcessorEditor* APTTiltProcessor::createEditor()
{
    return new APTTiltEditor(*this);
}

void APTTiltProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTTiltProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTTiltProcessor();
}
