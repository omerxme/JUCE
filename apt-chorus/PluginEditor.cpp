#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

APTChorusEditor::APTChorusEditor(APTChorusProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 650);

    // Amount slider setup
    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    amountSlider.setRange(0.0, 1.0, 0.001);
    amountSlider.setValue(0.0);
    amountSlider.setDoubleClickReturnValue(true, 0.0);
    
    amountSlider.onValueChange = [this]()
    {
        repaint();
    };
    
    amountSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(amountSlider);

    // Attach parameter
    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "amount", amountSlider);
    
    // Preset selector setup
    presetSelector.setTextWhenNothingSelected("Select Preset...");
    presetSelector.setTextWhenNoChoicesAvailable("No Presets");
    
    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
    presetSelector.setColour(juce::ComboBox::textColourId, juce::Colour(0xffd4c5a9));
    presetSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff555555));
    presetSelector.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffd4c5a9));
    
    for (int i = 0; i < presetManager.getNumPresets(); ++i)
    {
        presetSelector.addItem(presetManager.getPresetName(i), i + 1);
    }
    
    presetSelector.onChange = [this]()
    {
        int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
        {
            presetManager.loadPreset(selectedId - 1, audioProcessor.getAPVTS());
        }
    };
    
    addAndMakeVisible(presetSelector);
    
    // Load logo
    int logoDataSize = 0;
    const char* logoData = BinaryData::getNamedResource("logo_png", logoDataSize);
    if (logoData != nullptr && logoDataSize > 0)
    {
        logoImage = juce::ImageFileFormat::loadFrom(logoData, logoDataSize);
    }
    
    startTimerHz(60); // Increased to 60 Hz for smoother animation
}

APTChorusEditor::~APTChorusEditor() 
{
    stopTimer();
    amountSlider.setLookAndFeel(nullptr);
}

void APTChorusEditor::drawScrew(juce::Graphics& g, float x, float y)
{
    float screwRadius = 12.0f;
    
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(x - screwRadius + 1, y - screwRadius + 1, screwRadius * 2, screwRadius * 2);
    
    juce::ColourGradient screwGradient(
        juce::Colour(0xff4a4a4a), x, y - screwRadius,
        juce::Colour(0xff2a2a2a), x, y + screwRadius,
        false);
    g.setGradientFill(screwGradient);
    g.fillEllipse(x - screwRadius, y - screwRadius, screwRadius * 2, screwRadius * 2);
    
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(x - screwRadius * 0.7f, y - 1.5f, screwRadius * 1.4f, 3.0f);
}

void APTChorusEditor::paint(juce::Graphics& g)
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
    
    // Draw glow ring around knob (CYAN/BLUE/PURPLE theme)
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY() + 30;
    float knobRadius = 140.0f;
    
    float amount = static_cast<float>(amountSlider.getValue());
    float glowIntensity = amount;
    
    if (glowIntensity > 0.01f)
    {
        // Chorus colors: Cyan → Blue → Purple
        juce::Colour baseColor, midColor, hotColor;
        
        if (amount < 0.5f)
        {
            // 0-50%: Cyan to Blue
            float t = amount * 2.0f;
            baseColor = juce::Colour(0xff00CED1).interpolatedWith(juce::Colour(0xff4169E1), t); // Cyan to Royal Blue
            midColor = juce::Colour(0xff00FFFF).interpolatedWith(juce::Colour(0xff6495ED), t);
            hotColor = juce::Colour(0xffAFEEEE).interpolatedWith(juce::Colour(0xff87CEEB), t);
        }
        else
        {
            // 50-100%: Blue to Purple
            float t = (amount - 0.5f) * 2.0f;
            baseColor = juce::Colour(0xff4169E1).interpolatedWith(juce::Colour(0xff9370DB), t); // Royal Blue to Purple
            midColor = juce::Colour(0xff6495ED).interpolatedWith(juce::Colour(0xffBA55D3), t);
            hotColor = juce::Colour(0xff87CEEB).interpolatedWith(juce::Colour(0xffDDA0DD), t);
        }
        
        // Ultra-smooth outer glow
        for (int i = 80; i > 0; --i)
        {
            float layerProgress = i / 80.0f;
            float falloff = layerProgress * layerProgress * layerProgress * layerProgress;
            float alpha = glowIntensity * 0.025f * falloff;
            g.setColour(baseColor.withAlpha(alpha));
            float r = knobRadius + i * 3.5f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
        
        // Middle glow
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
        
        // Inner glow
        for (int i = 25; i > 0; --i)
        {
            float layerProgress = 1.0f - (i / 25.0f);
            
            juce::Colour glowColor = midColor.interpolatedWith(
                hotColor, glowIntensity * layerProgress * 0.6f);
            
            float alpha = glowIntensity * (0.08f + layerProgress * 0.12f);
            g.setColour(glowColor.withAlpha(alpha));
            float r = knobRadius - 5 + i * 1.0f;
            g.fillEllipse(centerX - r, centerY - r, r * 2, r * 2);
        }
    }
    
    // Draw title
    g.setFont(juce::Font("Arial", 48.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("APT-CHORUS", bounds.getX(), 52, bounds.getWidth(), 80, 
               juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("APT-CHORUS", bounds.getX(), 50, bounds.getWidth(), 80, 
               juce::Justification::centred);
    
    // Draw "PRESETS" label
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("PRESETS", bounds.getX() + 21, 3, 80, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("PRESETS", bounds.getX() + 20, 2, 80, 20, juce::Justification::centredLeft);
    
    // Draw amount display below knob
    juce::String amountText = getAmountString(amount);
    int textY = bounds.getCentreY() + 180;
    
    g.setFont(juce::Font("Arial", 28.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
    g.drawText(amountText, bounds.getX(), textY + 2, bounds.getWidth(), 40, 
               juce::Justification::centred);
    
    // Color-coded text
    juce::Colour textColor;
    if (amount < 0.01f)
        textColor = juce::Colour(0xff888888);
    else if (amount < 0.5f)
        textColor = juce::Colour(0xff00CED1); // Cyan
    else
        textColor = juce::Colour(0xff9370DB); // Purple
    
    g.setColour(textColor);
    g.drawText(amountText, bounds.getX(), textY, bounds.getWidth(), 40, 
               juce::Justification::centred);
    
    // Draw "DRY" and "WET" labels
    g.setFont(juce::Font("Arial", 36.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("DRY", 30, bounds.getHeight() - 90, 100, 50, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("DRY", 29, bounds.getHeight() - 91, 100, 50, juce::Justification::centred);
    
    g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
    g.drawText("WET", bounds.getWidth() - 131, bounds.getHeight() - 90, 100, 50, 
               juce::Justification::centred);
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("WET", bounds.getWidth() - 130, bounds.getHeight() - 91, 100, 50, 
               juce::Justification::centred);
    
    // Draw logo
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

void APTChorusEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Preset selector at top
    presetSelector.setBounds(bounds.getX() + 20, bounds.getY() + 15, bounds.getWidth() - 40, 30);
    
    // Center the main knob
    int knobSize = 280;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    int knobY = (bounds.getHeight() - knobSize) / 2 + 30;
    
    amountSlider.setBounds(knobX, knobY, knobSize, knobSize);
}

void APTChorusEditor::timerCallback()
{
    repaint();
}

juce::String APTChorusEditor::getAmountString(float amount)
{
    if (amount < 0.01f)
        return "DRY";
    else
        return juce::String(static_cast<int>(amount * 100.0f)) + "%";
}
