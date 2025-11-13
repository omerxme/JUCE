#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

APTDriveEditor::APTDriveEditor(APTDriveProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 650);

    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    driveSlider.setRange(0.0, 1.0, 0.001);
    driveSlider.setValue(0.0);
    driveSlider.setDoubleClickReturnValue(true, 0.0);
    
    driveSlider.onValueChange = [this]() { repaint(); };
    driveSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(driveSlider);

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "drive", driveSlider);
    
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

APTDriveEditor::~APTDriveEditor() 
{
    stopTimer();
    driveSlider.setLookAndFeel(nullptr);
}

void APTDriveEditor::drawScrew(juce::Graphics& g, float x, float y)
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

void APTDriveEditor::paint(juce::Graphics& g)
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
    
    // AMBER/ORANGE/RED glow for drive
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY() + 30;
    float knobRadius = 140.0f;
    float drive = static_cast<float>(driveSlider.getValue());
    float glowIntensity = drive;
    
    if (glowIntensity > 0.01f)
    {
        juce::Colour baseColor, midColor, hotColor;
        
        if (drive < 0.4f)
        {
            // 0-40%: Amber
            float t = drive / 0.4f;
            baseColor = juce::Colour(0xffFFBF00).interpolatedWith(juce::Colour(0xffFF8C00), t);
            midColor = juce::Colour(0xffFFD700).interpolatedWith(juce::Colour(0xffFFA500), t);
            hotColor = juce::Colour(0xffFFE4B5).interpolatedWith(juce::Colour(0xffFFDAB9), t);
        }
        else if (drive < 0.8f)
        {
            // 40-80%: Orange
            float t = (drive - 0.4f) / 0.4f;
            baseColor = juce::Colour(0xffFF8C00).interpolatedWith(juce::Colour(0xffFF4500), t);
            midColor = juce::Colour(0xffFFA500).interpolatedWith(juce::Colour(0xffFF6347), t);
            hotColor = juce::Colour(0xffFFDAB9).interpolatedWith(juce::Colour(0xffFFB6C1), t);
        }
        else
        {
            // 80-100%: Warm Red
            float t = (drive - 0.8f) / 0.2f;
            baseColor = juce::Colour(0xffFF4500).interpolatedWith(juce::Colour(0xffDC143C), t);
            midColor = juce::Colour(0xffFF6347).interpolatedWith(juce::Colour(0xffFF69B4), t);
            hotColor = juce::Colour(0xffFFB6C1).interpolatedWith(juce::Colour(0xffFFE4E1), t);
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
    g.drawText("APT-DRIVE", bounds.getX(), 52, bounds.getWidth(), 80, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("APT-DRIVE", bounds.getX(), 50, bounds.getWidth(), 80, juce::Justification::centred);
    
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("PRESETS", bounds.getX() + 21, 3, 80, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("PRESETS", bounds.getX() + 20, 2, 80, 20, juce::Justification::centredLeft);
    
    juce::String driveText = getDriveString(drive);
    int textY = bounds.getCentreY() + 180;
    
    g.setFont(juce::Font("Arial", 28.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
    g.drawText(driveText, bounds.getX(), textY + 2, bounds.getWidth(), 40, juce::Justification::centred);
    
    juce::Colour textColor = (drive < 0.01f) ? juce::Colour(0xff888888) : 
                             (drive < 0.4f) ? juce::Colour(0xffFFBF00) :
                             (drive < 0.8f) ? juce::Colour(0xffFF8C00) : juce::Colour(0xffDC143C);
    
    g.setColour(textColor);
    g.drawText(driveText, bounds.getX(), textY, bounds.getWidth(), 40, juce::Justification::centred);
    
    g.setFont(juce::Font("Arial", 36.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("CLEAN", 41, bounds.getHeight() - 90, 80, 50, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("CLEAN", 40, bounds.getHeight() - 91, 80, 50, juce::Justification::centred);
    
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("DRIVE", bounds.getWidth() - 121, bounds.getHeight() - 90, 80, 50, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("DRIVE", bounds.getWidth() - 120, bounds.getHeight() - 91, 80, 50, juce::Justification::centred);
    
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

void APTDriveEditor::resized()
{
    auto bounds = getLocalBounds();
    presetSelector.setBounds(bounds.getX() + 20, bounds.getY() + 15, bounds.getWidth() - 40, 30);
    
    int knobSize = 280;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    int knobY = (bounds.getHeight() - knobSize) / 2 + 30;
    driveSlider.setBounds(knobX, knobY, knobSize, knobSize);
}

void APTDriveEditor::timerCallback()
{
    repaint();
}

juce::String APTDriveEditor::getDriveString(float drive)
{
    if (drive < 0.01f)
        return "CLEAN";
    else
        return juce::String(static_cast<int>(drive * 100.0f)) + "%";
}
