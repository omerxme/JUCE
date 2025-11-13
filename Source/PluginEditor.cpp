#include "PluginProcessor.h"
#include "PluginEditor.h"

APTFilterEditor::APTFilterEditor(APTFilterProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 600);

    // Filter slider setup
    filterSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filterSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    filterSlider.setRange(-1.0, 1.0, 0.001);
    filterSlider.setValue(0.0);
    filterSlider.setDoubleClickReturnValue(true, 0.0);
    
    // Add snap-to-center behavior
    filterSlider.onValueChange = [this]()
    {
        double value = filterSlider.getValue();
        const double snapThreshold = 0.05; // Snap zone around center
        
        if (std::abs(value) < snapThreshold)
        {
            filterSlider.setValue(0.0, juce::dontSendNotification);
        }
        
        // Trigger repaint for glow effect
        repaint();
    };
    
    filterSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(filterSlider);

    // Attach parameter
    filterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "filter", filterSlider);
    
    // Resonance slider setup (small knob)
    resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    resonanceSlider.setRange(0.5, 10.0, 0.01);
    resonanceSlider.setValue(0.707);
    resonanceSlider.setDoubleClickReturnValue(true, 0.707);
    resonanceSlider.setLookAndFeel(&vintageLookAndFeel);
    resonanceSlider.setTextValueSuffix(" Q");
    addAndMakeVisible(resonanceSlider);
    
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "resonance", resonanceSlider);
    
    // Start timer for smooth glow updates
    startTimerHz(30);
}

APTFilterEditor::~APTFilterEditor() 
{
    stopTimer();
    filterSlider.setLookAndFeel(nullptr);
    resonanceSlider.setLookAndFeel(nullptr);
}

void APTFilterEditor::drawScrew(juce::Graphics& g, float x, float y)
{
    float screwRadius = 12.0f;
    
    // Screw head shadow
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(x - screwRadius + 1, y - screwRadius + 1, screwRadius * 2, screwRadius * 2);
    
    // Screw head
    juce::ColourGradient screwGradient(
        juce::Colour(0xff4a4a4a), x, y - screwRadius,
        juce::Colour(0xff2a2a2a), x, y + screwRadius,
        false);
    g.setGradientFill(screwGradient);
    g.fillEllipse(x - screwRadius, y - screwRadius, screwRadius * 2, screwRadius * 2);
    
    // Screw slot
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(x - screwRadius * 0.7f, y - 1.5f, screwRadius * 1.4f, 3.0f);
}

void APTFilterEditor::drawMetalTexture(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Base metal color (dark bronze/rust)
    juce::ColourGradient bgGradient(
        juce::Colour(0xff3a2a1a), bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xff2a1a0a), bounds.getCentreX(), bounds.getBottom(),
        false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);
    
    // Add noise/texture
    juce::Random random(42);
    for (int i = 0; i < 2000; ++i)
    {
        float x = random.nextFloat() * bounds.getWidth();
        float y = random.nextFloat() * bounds.getHeight();
        float brightness = random.nextFloat() * 0.15f;
        
        g.setColour(juce::Colour(0xff000000).withAlpha(brightness));
        g.fillRect(x, y, 1.0f, 1.0f);
    }
    
    // Vignette effect
    juce::ColourGradient vignette(
        juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getCentreY(),
        juce::Colour(0xff000000).withAlpha(0.5f), bounds.getX(), bounds.getY(),
        true);
    g.setGradientFill(vignette);
    g.fillRect(bounds);
}

void APTFilterEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Dark gradient background
    juce::ColourGradient bgGradient(
        juce::Colour(0xff2a2a2a), bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xff1a1a1a), bounds.getCentreX(), bounds.getBottom(),
        false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds.toFloat(), 20.0f);
    
    // Draw color-coded zones (LP = blue, HP = red)
    drawColorCodedZones(g, bounds);
    
    // Draw corner screws
    drawScrew(g, 30, 30);
    drawScrew(g, bounds.getWidth() - 30, 30);
    drawScrew(g, 30, bounds.getHeight() - 30);
    drawScrew(g, bounds.getWidth() - 30, bounds.getHeight() - 30);
    
    // Draw realistic glow ring around knob area (like a light bulb)
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY() + 20;
    float knobRadius = 140.0f; // Knob radius
    
    // Get current filter value for glow intensity
    float filterValue = static_cast<float>(filterSlider.getValue());
    float glowIntensity = std::abs(filterValue);
    
    // Subtle pulsing effect (breathing)
    float pulseAmount = 1.0f + (std::sin(pulsePhase) * 0.08f); // ±8% variation
    float pulsedIntensity = glowIntensity * pulseAmount;
    
    // Only draw glow if filter is active
    if (glowIntensity > 0.01f)
    {
        // Outer glow - soft fade to darkness (like light spreading)
        for (int i = 50; i > 0; --i)
        {
            float layerProgress = i / 50.0f;
            float alpha = pulsedIntensity * 0.08f * layerProgress * layerProgress; // Quadratic falloff
            g.setColour(juce::Colour(0xffff8c00).withAlpha(alpha));
            float r = knobRadius + i * 5;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        // Middle glow - brighter orange
        for (int i = 20; i > 0; --i)
        {
            float layerProgress = i / 20.0f;
            float alpha = pulsedIntensity * 0.15f * layerProgress;
            g.setColour(juce::Colour(0xffff8c00).withAlpha(alpha));
            float r = knobRadius + i * 2;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        // Inner hot ring - transitions from orange to yellow to white (like heating metal)
        for (int i = 15; i > 0; --i)
        {
            float layerProgress = 1.0f - (i / 15.0f); // 0 at outer, 1 at inner
            
            // Color transition: orange -> yellow -> white based on intensity
            juce::Colour glowColor;
            if (glowIntensity < 0.5f)
            {
                // Low intensity: orange to yellow
                float t = glowIntensity * 2.0f; // 0 to 1
                glowColor = juce::Colour(0xffff8c00).interpolatedWith(juce::Colour(0xffffd700), t * layerProgress);
            }
            else
            {
                // High intensity: yellow to white
                float t = (glowIntensity - 0.5f) * 2.0f; // 0 to 1
                glowColor = juce::Colour(0xffffd700).interpolatedWith(juce::Colours::white, t * layerProgress);
            }
            
            float alpha = pulsedIntensity * (0.3f + layerProgress * 0.4f);
            g.setColour(glowColor.withAlpha(alpha));
            float r = knobRadius - 5 + i * 1.5f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
    }
    
    // Draw title "APT-FILTER" with modern style
    g.setFont(juce::Font("Arial", 48.0f, juce::Font::bold));
    
    // Shadow
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("APT-FILTER", bounds.getX(), 42, bounds.getWidth(), 80, 
               juce::Justification::centred);
    
    // Main text (beige/tan color)
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("APT-FILTER", bounds.getX(), 40, bounds.getWidth(), 80, 
               juce::Justification::centred);
    
    // Draw circular progress ring around knob
    drawCircularProgressRing(g, centerX, centerY, 140.0f);
    
    // Draw filter type indicator (above knob)
    drawFilterTypeIndicator(g, bounds);
    
    // Draw frequency display (below knob)
    drawFrequencyDisplay(g, bounds);
    
    // Draw LP and HP labels
    g.setFont(juce::Font("Arial", 36.0f, juce::Font::bold));
    
    // LP label (left) - smaller, at bottom
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("LP", 61, bounds.getHeight() - 90, 60, 50, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("LP", 60, bounds.getHeight() - 91, 60, 50, juce::Justification::centred);
    
    // HP label (right) - smaller, at bottom
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("HP", bounds.getWidth() - 121, bounds.getHeight() - 90, 60, 50, 
               juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("HP", bounds.getWidth() - 120, bounds.getHeight() - 91, 60, 50, 
               juce::Justification::centred);
}

void APTFilterEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Center the main filter knob
    int knobSize = 280;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    int knobY = (bounds.getHeight() - knobSize) / 2 + 20;
    
    filterSlider.setBounds(knobX, knobY, knobSize, knobSize);
    
    // Resonance knob (smaller, bottom right)
    int resKnobSize = 80;
    int resKnobX = bounds.getWidth() - resKnobSize - 40;
    int resKnobY = bounds.getHeight() - resKnobSize - 120;
    
    resonanceSlider.setBounds(resKnobX, resKnobY, resKnobSize, resKnobSize + 25);
}

void APTFilterEditor::timerCallback()
{
    // Update pulse phase for glow pulsing (very subtle)
    pulsePhase += 0.05f;
    if (pulsePhase > juce::MathConstants<float>::twoPi)
        pulsePhase -= juce::MathConstants<float>::twoPi;
    
    // Repaint for smooth glow animation
    repaint();
}

void APTFilterEditor::drawColorCodedZones(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    float filterValue = static_cast<float>(filterSlider.getValue());
    float centerX = static_cast<float>(bounds.getCentreX());
    float centerY = static_cast<float>(bounds.getCentreY() + 20);
    
    // Only draw zones when filter is active
    if (std::abs(filterValue) > 0.01f)
    {
        // Left zone (LP) - Blue gradient
        if (filterValue < -0.01f)
        {
            float intensity = std::abs(filterValue) * 0.15f;
            juce::ColourGradient leftGradient(
                juce::Colour(0xff4169e1).withAlpha(intensity), // Royal blue
                static_cast<float>(bounds.getX()), centerY,
                juce::Colours::transparentBlack,
                centerX, centerY,
                false);
            g.setGradientFill(leftGradient);
            g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth() / 2, bounds.getHeight());
        }
        // Right zone (HP) - Red gradient
        else if (filterValue > 0.01f)
        {
            float intensity = std::abs(filterValue) * 0.15f;
            juce::ColourGradient rightGradient(
                juce::Colours::transparentBlack,
                centerX, centerY,
                juce::Colour(0xffdc143c).withAlpha(intensity), // Crimson red
                static_cast<float>(bounds.getRight()), centerY,
                false);
            g.setGradientFill(rightGradient);
            g.fillRect(bounds.getCentreX(), bounds.getY(), bounds.getWidth() / 2, bounds.getHeight());
        }
    }
}

juce::String APTFilterEditor::getFrequencyString(float filterValue)
{
    const float threshold = 0.01f;
    
    if (filterValue < -threshold)
    {
        // Low-Pass Filter: map -1.0 → 0.0 to 200 Hz → 20000 Hz
        float normalizedValue = (filterValue + 1.0f);
        float cutoffFreq = 200.0f + normalizedValue * (20000.0f - 200.0f);
        
        if (cutoffFreq >= 1000.0f)
            return juce::String(cutoffFreq / 1000.0f, 1) + " kHz";
        else
            return juce::String((int)cutoffFreq) + " Hz";
    }
    else if (filterValue > threshold)
    {
        // High-Pass Filter: map 0.0 → 1.0 to 20 Hz → 1000 Hz
        float cutoffFreq = 20.0f + filterValue * (1000.0f - 20.0f);
        
        if (cutoffFreq >= 1000.0f)
            return juce::String(cutoffFreq / 1000.0f, 1) + " kHz";
        else
            return juce::String((int)cutoffFreq) + " Hz";
    }
    else
    {
        return "BYPASS";
    }
}

juce::String APTFilterEditor::getFilterTypeString(float filterValue)
{
    const float threshold = 0.01f;
    
    if (filterValue < -threshold)
        return "LOW PASS";
    else if (filterValue > threshold)
        return "HIGH PASS";
    else
        return "BYPASS";
}

void APTFilterEditor::drawFrequencyDisplay(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    float filterValue = static_cast<float>(filterSlider.getValue());
    juce::String freqText = getFrequencyString(filterValue);
    
    // Position below knob
    int textY = bounds.getCentreY() + 180;
    
    // Shadow
    g.setFont(juce::Font("Arial", 28.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
    g.drawText(freqText, bounds.getX(), textY + 2, bounds.getWidth(), 40, 
               juce::Justification::centred);
    
    // Main text with glow color
    float glowIntensity = std::abs(filterValue);
    juce::Colour textColor;
    
    if (glowIntensity < 0.01f)
    {
        textColor = juce::Colour(0xff888888); // Gray for bypass
    }
    else if (glowIntensity < 0.5f)
    {
        textColor = juce::Colour(0xffff8c00); // Orange
    }
    else
    {
        // Transition to yellow/white at high intensity
        float t = (glowIntensity - 0.5f) * 2.0f;
        textColor = juce::Colour(0xffff8c00).interpolatedWith(juce::Colour(0xffffd700), t);
    }
    
    g.setColour(textColor);
    g.drawText(freqText, bounds.getX(), textY, bounds.getWidth(), 40, 
               juce::Justification::centred);
}

void APTFilterEditor::drawFilterTypeIndicator(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    float filterValue = static_cast<float>(filterSlider.getValue());
    juce::String typeText = getFilterTypeString(filterValue);
    
    // Position above knob
    int textY = bounds.getCentreY() - 180;
    
    // Shadow
    g.setFont(juce::Font("Arial", 20.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText(typeText, bounds.getX(), textY + 2, bounds.getWidth(), 30, 
               juce::Justification::centred);
    
    // Main text
    float glowIntensity = std::abs(filterValue);
    juce::Colour textColor;
    
    if (glowIntensity < 0.01f)
    {
        textColor = juce::Colour(0xff666666); // Darker gray for bypass
    }
    else
    {
        textColor = juce::Colour(0xffd4c5a9); // Beige/tan
    }
    
    g.setColour(textColor);
    g.drawText(typeText, bounds.getX(), textY, bounds.getWidth(), 30, 
               juce::Justification::centred);
}

void APTFilterEditor::drawCircularProgressRing(juce::Graphics& g, float centerX, float centerY, float radius)
{
    float filterValue = static_cast<float>(filterSlider.getValue());
    
    // Ring parameters - closer to knob, like in the image
    float ringRadius = radius - 10.0f; // Inside the knob area
    float ringThickness = 8.0f; // Thicker for visibility
    float dotRadius = 7.0f;
    
    // Calculate knob position angle (matches the white indicator line on knob)
    // Map -1.0 to +1.0 → -135° to +135° (270 degree range)
    float centerAngle = -juce::MathConstants<float>::pi / 2.0f; // Top position (12 o'clock)
    float knobAngle = centerAngle + (filterValue * juce::MathConstants<float>::pi * 0.75f);
    
    // Draw filled arc from center to knob position (like in the image)
    if (std::abs(filterValue) > 0.01f)
    {
        juce::Colour arcColor;
        float glowIntensity = std::abs(filterValue);
        
        if (glowIntensity < 0.5f)
        {
            arcColor = juce::Colour(0xffff8c00); // Orange
        }
        else
        {
            float t = (glowIntensity - 0.5f) * 2.0f;
            arcColor = juce::Colour(0xffff8c00).interpolatedWith(juce::Colour(0xffffd700), t);
        }
        
        // Draw filled arc - from center position to knob position
        g.setColour(arcColor.withAlpha(0.8f));
        juce::Path filledArc;
        
        if (filterValue < 0)
        {
            // Left side (LP) - counter-clockwise from center
            filledArc.addCentredArc(centerX, centerY, ringRadius, ringRadius, 
                                   0.0f, knobAngle, centerAngle, true);
        }
        else
        {
            // Right side (HP) - clockwise from center
            filledArc.addCentredArc(centerX, centerY, ringRadius, ringRadius, 
                                   0.0f, centerAngle, knobAngle, true);
        }
        
        g.strokePath(filledArc, juce::PathStrokeType(ringThickness, 
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    // Calculate dot position at the end of the arc
    float dotX = centerX + ringRadius * std::cos(knobAngle);
    float dotY = centerY + ringRadius * std::sin(knobAngle);
    
    // Draw colored dot at knob position (matches the image)
    if (std::abs(filterValue) > 0.01f)
    {
        juce::Colour dotColor;
        float glowIntensity = std::abs(filterValue);
        
        if (glowIntensity < 0.5f)
        {
            dotColor = juce::Colour(0xffff8c00); // Orange
        }
        else
        {
            float t = (glowIntensity - 0.5f) * 2.0f;
            dotColor = juce::Colour(0xffff8c00).interpolatedWith(juce::Colour(0xffffd700), t);
        }
        
        // Outer glow
        g.setColour(dotColor.withAlpha(0.4f));
        g.fillEllipse(dotX - dotRadius * 1.8f, dotY - dotRadius * 1.8f, 
                     dotRadius * 3.6f, dotRadius * 3.6f);
        
        // Main dot
        g.setColour(dotColor);
        g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
        
        // Bright highlight
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillEllipse(dotX - dotRadius * 0.4f, dotY - dotRadius * 0.6f, 
                     dotRadius * 0.8f, dotRadius * 0.8f);
    }
}
