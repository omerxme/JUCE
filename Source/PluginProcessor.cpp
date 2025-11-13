#include "PluginProcessor.h"
#include "PluginEditor.h"

APTFilterProcessor::APTFilterProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedFilterValue.setCurrentAndTargetValue(0.0f);
}

APTFilterProcessor::~APTFilterProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTFilterProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "filter",
        "Filter",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < -0.01f)
                return "LPF " + juce::String(std::abs(value) * 100.0f, 0) + "%";
            else if (value > 0.01f)
                return "HPF " + juce::String(value * 100.0f, 0) + "%";
            else
                return juce::String("Bypass");
        }));

    return layout;
}

void APTFilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    leftFilter.prepare(spec);
    rightFilter.prepare(spec);
    
    smoothedFilterValue.reset(sampleRate, 0.05); // 50ms smoothing
    
    updateFilter(0.0f, sampleRate);
}

void APTFilterProcessor::releaseResources() {}

bool APTFilterProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void APTFilterProcessor::updateFilter(float filterValue, double sampleRate)
{
    const float threshold = 0.01f;
    
    if (filterValue < -threshold)
    {
        // Low-Pass Filter: map -1.0 → 0.0 to 200 Hz → 20000 Hz
        float normalizedValue = (filterValue + 1.0f); // 0.0 to 1.0
        float cutoffFreq = 200.0f + normalizedValue * (20000.0f - 200.0f);
        cutoffFreq = juce::jlimit(200.0f, 20000.0f, cutoffFreq);
        
        *leftFilter.state = *FilterCoefs::makeLowPass(sampleRate, cutoffFreq, 0.707f);
        *rightFilter.state = *FilterCoefs::makeLowPass(sampleRate, cutoffFreq, 0.707f);
    }
    else if (filterValue > threshold)
    {
        // High-Pass Filter: map 0.0 → 1.0 to 20 Hz → 1000 Hz
        float cutoffFreq = 20.0f + filterValue * (1000.0f - 20.0f);
        cutoffFreq = juce::jlimit(20.0f, 1000.0f, cutoffFreq);
        
        *leftFilter.state = *FilterCoefs::makeHighPass(sampleRate, cutoffFreq, 0.707f);
        *rightFilter.state = *FilterCoefs::makeHighPass(sampleRate, cutoffFreq, 0.707f);
    }
    else
    {
        // Bypass: create all-pass filter (unity gain at all frequencies)
        *leftFilter.state = *FilterCoefs::makeAllPass(sampleRate, 1000.0f);
        *rightFilter.state = *FilterCoefs::makeAllPass(sampleRate, 1000.0f);
    }
}

void APTFilterProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get parameter value and set target for smoothing
    float targetFilterValue = *apvts.getRawParameterValue("filter");
    smoothedFilterValue.setTargetValue(targetFilterValue);
    
    // Update filter coefficients at the start of the block
    float currentFilterValue = smoothedFilterValue.getCurrentValue();
    
    const float threshold = 0.05f; // Wider threshold for smoother transition
    const float fadeZone = 0.1f;   // Fade zone around bypass
    
    // Calculate mix amount (0.0 = bypass, 1.0 = full filter)
    float mixAmount = 1.0f;
    if (std::abs(currentFilterValue) < fadeZone)
    {
        mixAmount = std::abs(currentFilterValue) / fadeZone;
    }
    
    bool shouldProcess = std::abs(currentFilterValue) > threshold;
    
    if (shouldProcess)
    {
        // Create a copy of the dry signal for crossfading
        juce::AudioBuffer<float> dryBuffer;
        dryBuffer.makeCopyOf(buffer);
        
        // Update and apply filter
        updateFilter(currentFilterValue, getSampleRate());
        
        juce::dsp::AudioBlock<float> block(buffer);
        
        // Process left channel
        if (totalNumInputChannels > 0)
        {
            auto leftBlock = block.getSingleChannelBlock(0);
            juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
            leftFilter.process(leftContext);
        }
        
        // Process right channel
        if (totalNumInputChannels > 1)
        {
            auto rightBlock = block.getSingleChannelBlock(1);
            juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
            rightFilter.process(rightContext);
        }
        
        // Crossfade between dry and wet near bypass zone
        if (mixAmount < 1.0f)
        {
            for (int channel = 0; channel < totalNumInputChannels; ++channel)
            {
                auto* wetData = buffer.getWritePointer(channel);
                auto* dryData = dryBuffer.getReadPointer(channel);
                
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    wetData[sample] = dryData[sample] * (1.0f - mixAmount) + wetData[sample] * mixAmount;
                }
            }
        }
    }
    // If not processing, audio passes through unchanged
    
    // Advance the smoothed value for next block
    smoothedFilterValue.skip(buffer.getNumSamples());
}

juce::AudioProcessorEditor* APTFilterProcessor::createEditor()
{
    return new APTFilterEditor(*this);
}

void APTFilterProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTFilterProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTFilterProcessor();
}
