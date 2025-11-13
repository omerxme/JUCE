#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

APTWideEditor::APTWideEditor(APTWideProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 650);

    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    widthSlider.setRange(-1.0, 1.0, 0.001);
    widthSlider.setValue(0.0);
    widthSlider.setDoubleClickReturnValue(true, 0.0);
    
    widthSlider.onValueChange = [this]() {
        double value = widthSlider.getValue();
        const double snapThreshold = 0.05;
        if (std::abs(value) < snapThreshold)
            widthSlider.setValue(0.0, juce::dontSendNotification);
        repaint();
    };
    
    widthSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(widthSlider);

    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "width", widthSlider);
    
    presetSelector.setTextWhenNothingSelected("Select Preset...");
    presetSelector.setTextWhenNoChoicesAvailable("No Presets");
    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
    presetSelector.setColour(juce::ComboBox::textColourId, juce::Colour(0xffd4c5a9));
    presetSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff555555));
    presetSelector.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffd4c5a9));
    
    for (int i = 0; i < presetManager.getNumPresets(); ++i)
        presetSelector.addItem(presetManager.getPresetName(i), i + 1);
    
    presetSelector.onChange = [this]() {
        int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
            presetManager.loadPreset(selectedId - 1, audioProcessor.getAPVTS());
    };
    
    addAndMakeVisible(presetSelector);
    
    int logoDataSize = 0;
    const char* logoData = BinaryData::getNamedResource("logo_png", logoDataSize);
    if (logoData != nullptr && logoDataSize > 0)
        logoImage = juce::ImageFileFormat::loadFrom(logoData, logoDataSize);
    
    startTimerHz(30);
}

APTWideEditor::~APTWideEditor() 
{
    stopTimer();
    widthSlider.setLookAndFeel(nullptr);
}

void APTWideEditor::drawScrew(juce::Graphics& g, float x, float y)
{
    float screwRadius = 12.0f;
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(x - screwRadius + 1, y - screwRadius + 1, screwRadius * 2, screwRadius * 2);
    
    juce::ColourGradient screwGradient(
        juce::Colour(0xff4a4a4a), x, y - screwRadius,
        juce::Colour(0xff2a2a2a), x, y + screwRadius, false);
    g.setGradientFill(screwGradient);
    g.fillEllipse(x - screwRadius, y - screwRadius, screwRadius * 2, screwRadius * 2);
    
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(x - screwRadius * 0.7f, y - 1.5f, screwRadius * 1.4f, 3.0f);
}

void APTWideEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    juce::ColourGradient bgGradient(
        juce::Colour(0xff2a2a2a), bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xff1a1a1a), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds.toFloat(), 20.0f);
    
    drawScrew(g, 30, 30);
    drawScrew(g, bounds.getWidth() - 30, 30);
    drawScrew(g, 30, bounds.getHeight() - 30);
    drawScrew(g, bounds.getWidth() - 30, bounds.getHeight() - 30);
    
    // RED (mono) / GRAY (neutral) / CYAN (wide) glow
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY() + 30;
    float knobRadius = 140.0f;
    float width = static_cast<float>(widthSlider.getValue());
    float glowIntensity = std::abs(width);
    
    if (glowIntensity > 0.01f)
    {
        juce::Colour baseColor, midColor, hotColor;
        
        if (width < 0.0f)
        {
            // Mono/Narrow: Red tones
            baseColor = juce::Colour(0xffDC143C);
            midColor = juce::Colour(0xffFF6347);
            hotColor = juce::Colour(0xffFF69B4);
        }
        else
        {
            // Wide: Cyan tones
            baseColor = juce::Colour(0xff00CED1);
            midColor = juce::Colour(0xff00FFFF);
            hotColor = juce::Colour(0xffAFEEEE);
        }
        
        for (int i = 80; i > 0; --i)
        {
            float layerProgress = i / 80.0f;
            float falloff = layerProgress * layerProgress * layerProgress * layerProgress;
            float alpha = glowIntensity * 0.025f * falloff;
            g.setColour(baseColor.withAlpha(alpha));
            float r = knobRadius + i * 3.5f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        for (int i = 40; i > 0; --i)
        {
            float layerProgress = i / 40.0f;
            float falloff = layerProgress * layerProgress * layerProgress;
            float alpha = glowIntensity * 0.04f * falloff;
            juce::Colour glowColor = baseColor.interpolatedWith(midColor, (1.0f - layerProgress) * 0.5f);
            g.setColour(glowColor.withAlpha(alpha));
            float r = knobRadius + i * 1.8f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        for (int i = 25; i > 0; --i)
        {
            float layerProgress = 1.0f - (i / 25.0f);
            juce::Colour glowColor = midColor.interpolatedWith(hotColor, glowIntensity * layerProgress * 0.6f);
            float alpha = glowIntensity * (0.08f + layerProgress * 0.12f);
            g.setColour(glowColor.withAlpha(alpha));
            float r = knobRadius - 5 + i * 1.0f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
    }
    
    g.setFont(juce::Font("Arial", 48.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("APT-WIDE", bounds.getX(), 52, bounds.getWidth(), 80, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("APT-WIDE", bounds.getX(), 50, bounds.getWidth(), 80, juce::Justification::centred);
    
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("PRESETS", bounds.getX() + 21, 3, 80, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("PRESETS", bounds.getX() + 20, 2, 80, 20, juce::Justification::centredLeft);
    
    juce::String widthText = getWidthString(width);
    int textY = bounds.getCentreY() + 180;
    
    g.setFont(juce::Font("Arial", 28.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
    g.drawText(widthText, bounds.getX(), textY + 2, bounds.getWidth(), 40, juce::Justification::centred);
    
    juce::Colour textColor = (std::abs(width) < 0.01f) ? juce::Colour(0xff888888) :
                             (width < 0.0f) ? juce::Colour(0xffDC143C) : juce::Colour(0xff00CED1);
    
    g.setColour(textColor);
    g.drawText(widthText, bounds.getX(), textY, bounds.getWidth(), 40, juce::Justification::centred);
    
    g.setFont(juce::Font("Arial", 36.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("MONO", 25, bounds.getHeight() - 90, 105, 50, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("MONO", 24, bounds.getHeight() - 91, 105, 50, juce::Justification::centred);
    
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("WIDE", bounds.getWidth() - 131, bounds.getHeight() - 90, 105, 50, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("WIDE", bounds.getWidth() - 130, bounds.getHeight() - 91, 105, 50, juce::Justification::centred);
    
    if (logoImage.isValid())
    {
        int logoHeight = 40;
        float aspectRatio = logoImage.getWidth() / (float)logoImage.getHeight();
        int logoWidth = (int)(logoHeight * aspectRatio);
        int logoX = (bounds.getWidth() - logoWidth) / 2;
        int logoY = bounds.getHeight() - logoHeight - 25;
        g.setOpacity(0.7f);
        g.drawImage(logoImage, logoX, logoY, logoWidth, logoHeight,
                   0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
}

void APTWideEditor::resized()
{
    auto bounds = getLocalBounds();
    presetSelector.setBounds(bounds.getX() + 20, bounds.getY() + 15, bounds.getWidth() - 40, 30);
    
    int knobSize = 280;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    int knobY = (bounds.getHeight() - knobSize) / 2 + 30;
    widthSlider.setBounds(knobX, knobY, knobSize, knobSize);
}

void APTWideEditor::timerCallback()
{
    repaint();
}

juce::String APTWideEditor::getWidthString(float width)
{
    if (std::abs(width) < 0.01f)
        return "NATURAL";
    else if (width < 0.0f)
        return "MONO " + juce::String(static_cast<int>(std::abs(width) * 100.0f)) + "%";
    else
        return "WIDE " + juce::String(static_cast<int>(width * 100.0f)) + "%";
}
