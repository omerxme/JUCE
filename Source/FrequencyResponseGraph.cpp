#include "FrequencyResponseGraph.h"

FrequencyResponseGraph::FrequencyResponseGraph()
{
    magnitudes.resize(200, 0.0f);
    calculateMagnitudes();
    
    // Make sure this component doesn't intercept mouse events
    setInterceptsMouseClicks(false, false);
}

void FrequencyResponseGraph::updateResponse(float filterValue, float resonance, double sampleRate)
{
    currentFilterValue = filterValue;
    currentResonance = resonance;
    currentSampleRate = sampleRate;
    calculateMagnitudes();
    repaint();
}

void FrequencyResponseGraph::calculateMagnitudes()
{
    const float threshold = 0.01f;
    const int numPoints = static_cast<int>(magnitudes.size());
    
    for (int i = 0; i < numPoints; ++i)
    {
        // Logarithmic frequency scale (20 Hz to 20 kHz)
        float t = i / static_cast<float>(numPoints - 1);
        float frequency = 20.0f * std::pow(1000.0f, t);
        
        magnitudes[i] = getMagnitudeForFrequency(frequency);
    }
}

float FrequencyResponseGraph::getMagnitudeForFrequency(float frequency)
{
    const float threshold = 0.01f;
    
    if (std::abs(currentFilterValue) < threshold)
    {
        return 1.0f; // Bypass - flat response
    }
    
    float cutoffFreq;
    bool isLowPass = currentFilterValue < 0.0f;
    
    if (isLowPass)
    {
        // Low-Pass Filter
        float normalizedValue = (currentFilterValue + 1.0f);
        cutoffFreq = 200.0f + normalizedValue * (20000.0f - 200.0f);
    }
    else
    {
        // High-Pass Filter
        cutoffFreq = 20.0f + currentFilterValue * (1000.0f - 20.0f);
    }
    
    // Simple 2nd order filter magnitude calculation
    float ratio = frequency / cutoffFreq;
    float q = currentResonance;
    
    float magnitude;
    if (isLowPass)
    {
        // Low-pass magnitude response
        float denominator = std::sqrt(1.0f + std::pow(ratio / q, 2.0f) + std::pow(ratio, 4.0f));
        magnitude = 1.0f / denominator;
    }
    else
    {
        // High-pass magnitude response
        float numerator = std::pow(ratio, 2.0f);
        float denominator = std::sqrt(1.0f + std::pow(ratio / q, 2.0f) + std::pow(ratio, 4.0f));
        magnitude = numerator / denominator;
    }
    
    return magnitude;
}

void FrequencyResponseGraph::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Very subtle background (almost transparent)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.15f));
    g.fillRoundedRectangle(bounds, 5.0f);
    
    // Draw grid lines (more subtle)
    g.setColour(juce::Colour(0xff333333).withAlpha(0.2f));
    
    // Horizontal grid (dB lines)
    for (int i = 0; i <= 4; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() * i / 4.0f);
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 0.5f);
    }
    
    // Vertical grid (frequency lines)
    for (int i = 0; i <= 4; ++i)
    {
        float x = bounds.getX() + (bounds.getWidth() * i / 4.0f);
        g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 0.5f);
    }
    
    // Draw frequency response curve
    if (magnitudes.size() > 1)
    {
        juce::Path responsePath;
        
        // Determine curve color based on filter type
        juce::Colour curveColor;
        if (std::abs(currentFilterValue) < 0.01f)
        {
            curveColor = juce::Colour(0xff888888); // Gray for bypass
        }
        else if (currentFilterValue < 0.0f)
        {
            curveColor = juce::Colour(0xff4da6ff); // Blue for LP
        }
        else
        {
            curveColor = juce::Colour(0xffff8c00); // Orange for HP
        }
        
        // Build path
        for (size_t i = 0; i < magnitudes.size(); ++i)
        {
            float x = bounds.getX() + (bounds.getWidth() * i / static_cast<float>(magnitudes.size() - 1));
            
            // Convert magnitude to dB and map to y position
            float magnitudeDB = 20.0f * std::log10(std::max(0.001f, magnitudes[i]));
            magnitudeDB = juce::jlimit(-48.0f, 12.0f, magnitudeDB); // Limit range
            
            // Map -48dB to bottom, +12dB to top
            float normalizedDB = (magnitudeDB + 48.0f) / 60.0f;
            float y = bounds.getBottom() - (bounds.getHeight() * normalizedDB);
            
            if (i == 0)
                responsePath.startNewSubPath(x, y);
            else
                responsePath.lineTo(x, y);
        }
        
        // Draw glow effect
        for (int i = 3; i > 0; --i)
        {
            g.setColour(curveColor.withAlpha(0.2f));
            g.strokePath(responsePath, juce::PathStrokeType(2.0f + i * 2.0f));
        }
        
        // Draw main curve
        g.setColour(curveColor.withAlpha(0.9f));
        g.strokePath(responsePath, juce::PathStrokeType(2.5f));
    }
    
    // Draw subtle border
    g.setColour(juce::Colour(0xff555555).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, 5.0f, 0.5f);
}
