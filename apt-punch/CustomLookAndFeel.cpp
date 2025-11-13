#include "CustomLookAndFeel.h"

VintageKnobLookAndFeel::VintageKnobLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff8c00));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff8b4513));
    setColour(juce::Slider::thumbColourId, juce::Colours::white);
}

void VintageKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centerX = bounds.getCentreX();
    auto centerY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Get slider value for glow intensity
    float value = (float)slider.getValue();
    float glowIntensity = std::abs(value);
    
    // Draw outer metal ring (bronze/brass - retro style)
    juce::ColourGradient outerRing(
        juce::Colour(0xff9b7653), centerX, centerY - radius,
        juce::Colour(0xff6b5033), centerX, centerY + radius,
        false);
    g.setGradientFill(outerRing);
    g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
    
    // Inner metal ring (darker brass)
    g.setColour(juce::Colour(0xff5b4023));
    g.fillEllipse(centerX - radius * 0.95f, centerY - radius * 0.95f, 
                 radius * 0.95f * 2, radius * 0.95f * 2);
    
    // Knob body - retro bakelite/plastic look (dark with slight gradient)
    juce::ColourGradient knobBody(
        juce::Colour(0xff1a1a1a), centerX, centerY - radius * 0.8f,
        juce::Colour(0xff0a0a0a), centerX, centerY + radius * 0.8f,
        false);
    g.setGradientFill(knobBody);
    g.fillEllipse(centerX - radius * 0.88f, centerY - radius * 0.88f, 
                 radius * 0.88f * 2, radius * 0.88f * 2);
    
    // Top surface with subtle highlight (vintage plastic sheen)
    juce::ColourGradient topSheen(
        juce::Colour(0xff2a2a2a).withAlpha(0.6f), centerX, centerY - radius * 0.6f,
        juce::Colours::transparentBlack, centerX, centerY + radius * 0.3f,
        false);
    g.setGradientFill(topSheen);
    g.fillEllipse(centerX - radius * 0.75f, centerY - radius * 0.75f, 
                 radius * 0.75f * 2, radius * 0.75f * 2);
    
    // Ribbed texture rings (vintage knob detail)
    for (int i = 0; i < 3; ++i)
    {
        float ringRadius = radius * (0.4f + i * 0.12f);
        g.setColour(juce::Colour(0xff000000).withAlpha(0.3f));
        g.drawEllipse(centerX - ringRadius, centerY - ringRadius, 
                     ringRadius * 2, ringRadius * 2, 1.0f);
    }
    
    // Indicator line (white, vintage style - thicker and shorter)
    juce::Path indicator;
    auto indicatorLength = radius * 0.55f;
    auto indicatorThickness = 5.0f;
    auto indicatorStart = radius * 0.15f;
    
    indicator.addRoundedRectangle(-indicatorThickness / 2, -indicatorLength + indicatorStart, 
                                  indicatorThickness, indicatorLength - indicatorStart, 2.5f);
    
    g.setColour(juce::Colours::white);
    g.fillPath(indicator, juce::AffineTransform::rotation(angle)
                                                 .translated(centerX, centerY));
    
    // Center cap (vintage screw/rivet look)
    juce::ColourGradient centerCap(
        juce::Colour(0xff3a3a3a), centerX, centerY - 8,
        juce::Colour(0xff1a1a1a), centerX, centerY + 8,
        false);
    g.setGradientFill(centerCap);
    g.fillEllipse(centerX - 8, centerY - 8, 16, 16);
    
    // Center cap highlight
    g.setColour(juce::Colour(0xff4a4a4a));
    g.fillEllipse(centerX - 7, centerY - 8, 14, 10);
    
    // Tiny center dot (screw detail)
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillEllipse(centerX - 2, centerY - 2, 4, 4);
}
