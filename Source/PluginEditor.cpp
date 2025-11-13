#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

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
    
    // Load logo from binary data
    int logoDataSize = 0;
    const char* logoData = BinaryData::getNamedResource("logo_png", logoDataSize);
    if (logoData != nullptr && logoDataSize > 0)
    {
        logoImage = juce::ImageFileFormat::loadFrom(logoData, logoDataSize);
    }
    
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
    
    // Only draw glow if filter is active
    if (glowIntensity > 0.01f)
    {
        // Determine glow color based on filter direction
        bool isLowPass = filterValue < 0.0f;
        
        // LP = Cool colors (blue/cyan), HP = Warm colors (orange/yellow)
        juce::Colour baseColor, midColor, hotColor;
        
        if (isLowPass)
        {
            // Low-Pass: Ice blue colors
            baseColor = juce::Colour(0xff4da6ff);  // Light blue
            midColor = juce::Colour(0xff66b3ff);   // Lighter blue
            hotColor = juce::Colour(0xffa3d5ff);   // Almost white blue
        }
        else
        {
            // High-Pass: Warm orange colors
            baseColor = juce::Colour(0xffff8c00);  // Orange
            midColor = juce::Colour(0xffffa500);   // Light orange
            hotColor = juce::Colour(0xffffd700);   // Gold/yellow
        }
        
        // Ultra-smooth outer glow - maximum blur effect
        for (int i = 80; i > 0; --i)
        {
            float layerProgress = i / 80.0f;
            // Quartic falloff for ultra-smooth fade
            float falloff = layerProgress * layerProgress * layerProgress * layerProgress;
            float alpha = glowIntensity * 0.025f * falloff;
            g.setColour(baseColor.withAlpha(alpha));
            float r = knobRadius + i * 3.5f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        // Middle glow - very smooth, low opacity
        for (int i = 40; i > 0; --i)
        {
            float layerProgress = i / 40.0f;
            float falloff = layerProgress * layerProgress * layerProgress;
            float alpha = glowIntensity * 0.04f * falloff;
            
            // Subtle color transition
            juce::Colour glowColor = baseColor.interpolatedWith(midColor, (1.0f - layerProgress) * 0.5f);
            g.setColour(glowColor.withAlpha(alpha));
            float r = knobRadius + i * 1.8f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        // Inner glow - color transition to hottest color
        for (int i = 25; i > 0; --i)
        {
            float layerProgress = 1.0f - (i / 25.0f);
            
            // Transition from mid to hot color based on intensity and layer
            juce::Colour glowColor = midColor.interpolatedWith(
                hotColor, glowIntensity * layerProgress * 0.6f);
            
            float alpha = glowIntensity * (0.08f + layerProgress * 0.12f);
            g.setColour(glowColor.withAlpha(alpha));
            float r = knobRadius - 5 + i * 1.0f;
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
    
    // Draw filter type indicator (above knob)
    drawFilterTypeIndicator(g, bounds);
    
    // Draw frequency display (below knob)
    drawFrequencyDisplay(g, bounds);
    
    // Draw control labels
    g.setFont(juce::Font("Arial", 16.0f, juce::Font::bold));
    
    // Resonance label (above knob, right side)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("RES", bounds.getWidth() - 139, bounds.getHeight() - 180, 80, 25, 
               juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("RES", bounds.getWidth() - 140, bounds.getHeight() - 181, 80, 25, 
               juce::Justification::centred);
    
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
    
    // Draw logo at bottom center (screw level)
    if (logoImage.isValid())
    {
        int logoHeight = 40; // Max height
        float aspectRatio = logoImage.getWidth() / (float)logoImage.getHeight();
        int logoWidth = (int)(logoHeight * aspectRatio);
        
        int logoX = (bounds.getWidth() - logoWidth) / 2;
        int logoY = bounds.getHeight() - logoHeight - 25; // Screw level
        
        // Draw with slight transparency to blend with background
        g.setOpacity(0.7f);
        g.drawImage(logoImage, logoX, logoY, logoWidth, logoHeight,
                   0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
}

void APTFilterEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Center the main filter knob
    int knobSize = 280;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    int knobY = (bounds.getHeight() - knobSize) / 2 + 20;
    
    filterSlider.setBounds(knobX, knobY, knobSize, knobSize);
    
    // Resonance knob (smaller, bottom right - more space for text)
    int resKnobSize = 80;
    int resKnobX = bounds.getWidth() - resKnobSize - 60;
    int resKnobY = bounds.getHeight() - resKnobSize - 140;
    
    resonanceSlider.setBounds(resKnobX, resKnobY, resKnobSize, resKnobSize + 30);
}

void APTFilterEditor::timerCallback()
{
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
    
    // Main text with glow color (matches filter direction)
    float glowIntensity = std::abs(filterValue);
    juce::Colour textColor;
    
    if (glowIntensity < 0.01f)
    {
        textColor = juce::Colour(0xff888888); // Gray for bypass
    }
    else if (filterValue < 0.0f)
    {
        // Low-Pass: Blue colors
        if (glowIntensity < 0.5f)
        {
            textColor = juce::Colour(0xff4da6ff); // Light blue
        }
        else
        {
            float t = (glowIntensity - 0.5f) * 2.0f;
            textColor = juce::Colour(0xff4da6ff).interpolatedWith(juce::Colour(0xffa3d5ff), t);
        }
    }
    else
    {
        // High-Pass: Orange colors
        if (glowIntensity < 0.5f)
        {
            textColor = juce::Colour(0xffff8c00); // Orange
        }
        else
        {
            float t = (glowIntensity - 0.5f) * 2.0f;
            textColor = juce::Colour(0xffff8c00).interpolatedWith(juce::Colour(0xffffd700), t);
        }
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

