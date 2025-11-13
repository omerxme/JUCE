#include "PluginProcessor.h"
#include "PluginEditor.h"

APTChorusProcessor::APTChorusProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedAmount.setCurrentAndTargetValue(0.0f);
}

APTChorusProcessor::~APTChorusProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTChorusProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "amount",
        "Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < 0.01f)
                return juce::String("Dry");
            else
                return juce::String(static_cast<int>(value * 100.0f)) + "%";
        }));

    return layout;
}

void APTChorusProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    delayLineLeft.prepare(spec);
    delayLineRight.prepare(spec);
    
    delayLineLeft.reset();
    delayLineRight.reset();
    
    smoothedAmount.reset(sampleRate, 0.05); // 50ms smoothing
    
    lfoPhase = 0.0f;
    lfoPhaseRight = juce::MathConstants<float>::pi * 0.5f;
}

void APTChorusProcessor::releaseResources() {}

bool APTChorusProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void APTChorusProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get parameter value
    float targetAmount = *apvts.getRawParameterValue("amount");
    smoothedAmount.setTargetValue(targetAmount);
    
    // If amount is very low, just pass through
    if (targetAmount < 0.01f)
    {
        smoothedAmount.skip(buffer.getNumSamples());
        return;
    }
    
    // Scale amount so 100% knob = 70% actual effect
    float scaledAmount = targetAmount * 0.7f;
    
    // Calculate chorus parameters based on amount
    // 0-50%: Low depth, wide stereo, slow rate
    // 50-100%: Depth increases, rate slightly faster
    float depth, rate, mix;
    
    if (scaledAmount <= 0.35f)
    {
        // First half: gentle, wide stereo
        float t = scaledAmount / 0.35f; // 0-1
        depth = 0.002f + t * 0.003f; // 2-5ms modulation
        rate = 0.3f + t * 0.2f; // 0.3-0.5 Hz
        mix = t * 0.4f; // 0-40% wet
    }
    else
    {
        // Second half: more pronounced
        float t = (scaledAmount - 0.35f) / 0.35f; // 0-1
        depth = 0.005f + t * 0.004f; // 5-9ms modulation
        rate = 0.5f + t * 0.25f; // 0.5-0.75 Hz (not too fast!)
        mix = 0.4f + t * 0.3f; // 40-70% wet
    }
    
    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = buffer.getWritePointer(1);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float currentAmount = smoothedAmount.getNextValue();
        float scaledCurrent = currentAmount * 0.7f;
        
        // Recalculate parameters for smooth transitions
        if (scaledCurrent <= 0.35f)
        {
            float t = scaledCurrent / 0.35f;
            depth = 0.002f + t * 0.003f;
            rate = 0.3f + t * 0.2f;
            mix = t * 0.4f;
        }
        else
        {
            float t = (scaledCurrent - 0.35f) / 0.35f;
            depth = 0.005f + t * 0.004f;
            rate = 0.5f + t * 0.25f;
            mix = 0.4f + t * 0.3f;
        }
        
        // Update LFO
        float lfoIncrement = rate * juce::MathConstants<float>::twoPi / static_cast<float>(currentSampleRate);
        lfoPhase += lfoIncrement;
        lfoPhaseRight += lfoIncrement;
        
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;
        if (lfoPhaseRight >= juce::MathConstants<float>::twoPi)
            lfoPhaseRight -= juce::MathConstants<float>::twoPi;
        
        // Calculate modulated delay times
        float lfoLeft = std::sin(lfoPhase);
        float lfoRight = std::sin(lfoPhaseRight);
        
        float delayTimeLeft = (10.0f + depth * 1000.0f * lfoLeft) * static_cast<float>(currentSampleRate) / 1000.0f;
        float delayTimeRight = (10.0f + depth * 1000.0f * lfoRight) * static_cast<float>(currentSampleRate) / 1000.0f;
        
        // Process left channel
        float dryLeft = leftData[sample];
        delayLineLeft.pushSample(0, dryLeft);
        float wetLeft = delayLineLeft.popSample(0, delayTimeLeft);
        leftData[sample] = dryLeft * (1.0f - mix) + wetLeft * mix;
        
        // Process right channel
        float dryRight = rightData[sample];
        delayLineRight.pushSample(0, dryRight);
        float wetRight = delayLineRight.popSample(0, delayTimeRight);
        rightData[sample] = dryRight * (1.0f - mix) + wetRight * mix;
    }
}

juce::AudioProcessorEditor* APTChorusProcessor::createEditor()
{
    return new APTChorusEditor(*this);
}

void APTChorusProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTChorusProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTChorusProcessor();
}
