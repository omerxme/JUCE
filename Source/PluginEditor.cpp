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
    
    // Start timer for smooth glow updates
    startTimerHz(30);
}

APTFilterEditor::~APTFilterEditor() 
{
    stopTimer();
    filterSlider.setLookAndFeel(nullptr);
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
        // Outer glow - soft fade to darkness (like light spreading)
        for (int i = 50; i > 0; --i)
        {
            float layerProgress = i / 50.0f;
            float alpha = glowIntensity * 0.08f * layerProgress * layerProgress; // Quadratic falloff
            g.setColour(juce::Colour(0xffff8c00).withAlpha(alpha));
            float r = knobRadius + i * 5;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        // Middle glow - brighter orange
        for (int i = 20; i > 0; --i)
        {
            float layerProgress = i / 20.0f;
            float alpha = glowIntensity * 0.15f * layerProgress;
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
            
            float alpha = glowIntensity * (0.3f + layerProgress * 0.4f);
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
    
    // Center the knob
    int knobSize = 280;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    int knobY = (bounds.getHeight() - knobSize) / 2 + 20;
    
    filterSlider.setBounds(knobX, knobY, knobSize, knobSize);
}

void APTFilterEditor::timerCallback()
{
    // Repaint for smooth glow animation
    repaint();
}
