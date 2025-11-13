#include "PluginProcessor.h"
#include "PluginEditor.h"

APTToolProcessor::APTToolProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    smoothedGain.setCurrentAndTargetValue(0.0f);
    smoothedBalance.setCurrentAndTargetValue(0.0f);
    smoothedWidth.setCurrentAndTargetValue(1.0f);
}

APTToolProcessor::~APTToolProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout APTToolProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Input section
    layout.add(std::make_unique<juce::AudioParameterBool>("phaseL", "Phase L", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("phaseR", "Phase R", false));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("stereoMode", "Stereo Mode",
        juce::StringArray{"Stereo", "Mono", "Swap"}, 0));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("width", "Width",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100.0f)) + "%"; }));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("mono", "Mono", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("bassMono", "Bass Mono", false));

    // Output section
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Gain",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f), 0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; }));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("balance", "Balance",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < -0.05f) return "L " + juce::String(static_cast<int>(std::abs(value) * 100.0f));
            else if (value > 0.05f) return "R " + juce::String(static_cast<int>(value * 100.0f));
            else return juce::String("C");
        }));
    
    layout.add(std::make_unique<juce::AudioParameterBool>("mute", "Mute", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("dcRemove", "DC Remove", false));

    return layout;
}

void APTToolProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;

    bassMonoFilterL.prepare(spec);
    bassMonoFilterR.prepare(spec);
    dcFilterL.prepare(spec);
    dcFilterR.prepare(spec);
    
    *bassMonoFilterL.state = *FilterCoefs::makeLowPass(sampleRate, bassMonoFreq, 0.707f);
    *bassMonoFilterR.state = *FilterCoefs::makeLowPass(sampleRate, bassMonoFreq, 0.707f);
    *dcFilterL.state = *FilterCoefs::makeHighPass(sampleRate, 5.0f, 0.707f);
    *dcFilterR.state = *FilterCoefs::makeHighPass(sampleRate, 5.0f, 0.707f);
    
    smoothedGain.reset(sampleRate, 0.05);
    smoothedBalance.reset(sampleRate, 0.05);
    smoothedWidth.reset(sampleRate, 0.05);
}

void APTToolProcessor::releaseResources() {}

bool APTToolProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void APTToolProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (totalNumInputChannels < 2)
        return;

    // Get parameters
    bool phaseL = *apvts.getRawParameterValue("phaseL");
    bool phaseR = *apvts.getRawParameterValue("phaseR");
    int stereoMode = static_cast<int>(*apvts.getRawParameterValue("stereoMode"));
    float width = *apvts.getRawParameterValue("width");
    bool mono = *apvts.getRawParameterValue("mono");
    bool bassMono = *apvts.getRawParameterValue("bassMono");
    float gain = *apvts.getRawParameterValue("gain");
    float balance = *apvts.getRawParameterValue("balance");
    bool mute = *apvts.getRawParameterValue("mute");
    bool dcRemove = *apvts.getRawParameterValue("dcRemove");

    smoothedGain.setTargetValue(gain);
    smoothedBalance.setTargetValue(balance);
    smoothedWidth.setTargetValue(width);

    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = buffer.getWritePointer(1);

    // INPUT PROCESSING
    
    // Phase invert
    if (phaseL)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            leftData[i] = -leftData[i];
    }
    if (phaseR)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            rightData[i] = -rightData[i];
    }

    // Stereo mode
    if (stereoMode == 1) // Mono
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float mono = (leftData[i] + rightData[i]) * 0.5f;
            leftData[i] = rightData[i] = mono;
        }
    }
    else if (stereoMode == 2) // Swap
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            std::swap(leftData[i], rightData[i]);
    }

    // Width
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float currentWidth = smoothedWidth.getNextValue();
        float mid = (leftData[i] + rightData[i]) * 0.5f;
        float side = (leftData[i] - rightData[i]) * 0.5f * currentWidth;
        leftData[i] = mid + side;
        rightData[i] = mid - side;
    }

    // Mono button
    if (mono)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float monoSig = (leftData[i] + rightData[i]) * 0.5f;
            leftData[i] = rightData[i] = monoSig;
        }
    }

    // Bass Mono
    if (bassMono)
    {
        juce::AudioBuffer<float> bassBufferL(1, buffer.getNumSamples());
        juce::AudioBuffer<float> bassBufferR(1, buffer.getNumSamples());
        
        bassBufferL.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
        bassBufferR.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
        
        juce::dsp::AudioBlock<float> blockL(bassBufferL);
        juce::dsp::AudioBlock<float> blockR(bassBufferR);
        juce::dsp::ProcessContextReplacing<float> contextL(blockL);
        juce::dsp::ProcessContextReplacing<float> contextR(blockR);
        
        bassMonoFilterL.process(contextL);
        bassMonoFilterR.process(contextR);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float bassL = bassBufferL.getSample(0, i);
            float bassR = bassBufferR.getSample(0, i);
            float bassMono = (bassL + bassR) * 0.5f;
            
            float highL = leftData[i] - bassL;
            float highR = rightData[i] - bassR;
            
            leftData[i] = bassMono + highL;
            rightData[i] = bassMono + highR;
        }
    }

    // OUTPUT PROCESSING
    
    // Gain
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float currentGain = juce::Decibels::decibelsToGain(smoothedGain.getNextValue());
        leftData[i] *= currentGain;
        rightData[i] *= currentGain;
    }

    // Balance
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float currentBalance = smoothedBalance.getNextValue();
        if (currentBalance < 0.0f) // Favor left
            rightData[i] *= (1.0f + currentBalance);
        else // Favor right
            leftData[i] *= (1.0f - currentBalance);
    }

    // DC Remove
    if (dcRemove)
    {
        juce::AudioBuffer<float> dcBufferL(1, buffer.getNumSamples());
        juce::AudioBuffer<float> dcBufferR(1, buffer.getNumSamples());
        
        dcBufferL.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
        dcBufferR.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
        
        juce::dsp::AudioBlock<float> blockL(dcBufferL);
        juce::dsp::AudioBlock<float> blockR(dcBufferR);
        juce::dsp::ProcessContextReplacing<float> contextL(blockL);
        juce::dsp::ProcessContextReplacing<float> contextR(blockR);
        
        dcFilterL.process(contextL);
        dcFilterR.process(contextR);
        
        buffer.copyFrom(0, 0, dcBufferL, 0, 0, buffer.getNumSamples());
        buffer.copyFrom(1, 0, dcBufferR, 0, 0, buffer.getNumSamples());
    }

    // Mute
    if (mute)
        buffer.clear();
}

juce::AudioProcessorEditor* APTToolProcessor::createEditor()
{
    return new APTToolEditor(*this);
}

void APTToolProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void APTToolProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APTToolProcessor();
}
