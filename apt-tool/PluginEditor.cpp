#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

APTToolEditor::APTToolEditor(APTToolProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 400);

    // Input section buttons
    setupButton(phaseLButton, "Ø L");
    setupButton(phaseRButton, "Ø R");
    setupButton(monoButton, "Mono");
    setupButton(bassMonoButton, "Bass Mono");
    
    // Stereo mode dropdown
    stereoModeBox.addItem("Stereo", 1);
    stereoModeBox.addItem("Mono", 2);
    stereoModeBox.addItem("Swap", 3);
    stereoModeBox.setSelectedId(1);
    stereoModeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
    stereoModeBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffd4c5a9));
    stereoModeBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff555555));
    addAndMakeVisible(stereoModeBox);
    
    // Width slider
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    widthSlider.setTextBoxIsEditable(false);
    widthSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(widthSlider);
    
    // Output section buttons
    setupButton(muteButton, "Mute");
    setupButton(dcButton, "DC");
    
    // Gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setTextBoxIsEditable(false);
    gainSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(gainSlider);
    
    // Balance slider
    balanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    balanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    balanceSlider.setTextBoxIsEditable(false);
    balanceSlider.setLookAndFeel(&vintageLookAndFeel);
    addAndMakeVisible(balanceSlider);
    
    // Attachments
    phaseLAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "phaseL", phaseLButton);
    phaseRAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "phaseR", phaseRButton);
    stereoModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "stereoMode", stereoModeBox);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "width", widthSlider);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "mono", monoButton);
    bassMonoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "bassMono", bassMonoButton);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gain", gainSlider);
    balanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "balance", balanceSlider);
    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "mute", muteButton);
    dcAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "dcRemove", dcButton);
    
    // Load logo
    int logoDataSize = 0;
    const char* logoData = BinaryData::getNamedResource("logo_png", logoDataSize);
    if (logoData != nullptr && logoDataSize > 0)
        logoImage = juce::ImageFileFormat::loadFrom(logoData, logoDataSize);
    
    startTimerHz(30);
}

APTToolEditor::~APTToolEditor() 
{
    stopTimer();
    widthSlider.setLookAndFeel(nullptr);
    gainSlider.setLookAndFeel(nullptr);
    balanceSlider.setLookAndFeel(nullptr);
}

void APTToolEditor::setupButton(juce::TextButton& button, const juce::String& text)
{
    button.setButtonText(text);
    button.setClickingTogglesState(true);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a2a));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff8800));
    button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd4c5a9));
    button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff000000));
    addAndMakeVisible(button);
}

void APTToolEditor::drawScrew(juce::Graphics& g, float x, float y)
{
    float screwRadius = 8.0f;
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

void APTToolEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Dark gradient background
    juce::ColourGradient bgGradient(
        juce::Colour(0xff2a2a2a), bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xff1a1a1a), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds.toFloat(), 15.0f);
    
    // Draw corner screws
    drawScrew(g, 20, 20);
    drawScrew(g, bounds.getWidth() - 20, 20);
    drawScrew(g, 20, bounds.getHeight() - 20);
    drawScrew(g, bounds.getWidth() - 20, bounds.getHeight() - 20);
    
    // Title bar
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(10, 10, bounds.getWidth() - 20, 50, 10.0f);
    
    // Orange indicator circle
    g.setColour(juce::Colour(0xffff8800));
    g.fillEllipse(25, 25, 20, 20);
    
    // Title
    g.setFont(juce::Font("Arial", 24.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("Utility", 55, 10, 150, 50, juce::Justification::centredLeft);
    
    // Hand icon placeholder (you can add actual icon later)
    g.setFont(juce::Font("Arial", 28.0f, juce::Font::plain));
    g.drawText("✋", 210, 10, 50, 50, juce::Justification::centred);
    
    // Divider line
    g.setColour(juce::Colour(0xff555555));
    g.fillRect(bounds.getCentreX() - 1, 80, 2, bounds.getHeight() - 160);
    
    // Section labels
    g.setFont(juce::Font("Arial", 20.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff888888));
    g.drawText("Input", 20, 70, 180, 30, juce::Justification::centred);
    g.drawText("Output", bounds.getWidth() - 200, 70, 180, 30, juce::Justification::centred);
    
    // Width label
    g.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffd4c5a9));
    g.drawText("Width", 60, 180, 100, 20, juce::Justification::centred);
    
    // Gain label
    g.drawText("Gain", bounds.getWidth() / 2 + 40, 120, 100, 20, juce::Justification::centred);
    
    // Balance label
    g.drawText("Balance", bounds.getWidth() / 2 + 40, 240, 100, 20, juce::Justification::centred);
    
    // Bass Mono frequency display
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff888888));
    g.drawText("120 Hz", 130, 310, 60, 20, juce::Justification::centred);
    
    // Logo
    if (logoImage.isValid())
    {
        int logoHeight = 30;
        float aspectRatio = logoImage.getWidth() / (float)logoImage.getHeight();
        int logoWidth = (int)(logoHeight * aspectRatio);
        int logoX = (bounds.getWidth() - logoWidth) / 2;
        int logoY = bounds.getHeight() - logoHeight - 15;
        g.setOpacity(0.5f);
        g.drawImage(logoImage, logoX, logoY, logoWidth, logoHeight,
                   0, 0, logoImage.getWidth(), logoImage.getHeight());
    }
}

void APTToolEditor::resized()
{
    auto bounds = getLocalBounds();
    int centerX = bounds.getCentreX();
    
    // Input section (left side)
    phaseLButton.setBounds(60, 110, 60, 30);
    phaseRButton.setBounds(130, 110, 60, 30);
    stereoModeBox.setBounds(60, 150, 130, 25);
    widthSlider.setBounds(60, 200, 100, 100);
    monoButton.setBounds(60, 310, 130, 30);
    bassMonoButton.setBounds(60, 350, 130, 30);
    
    // Output section (right side)
    gainSlider.setBounds(centerX + 40, 140, 100, 100);
    balanceSlider.setBounds(centerX + 40, 260, 100, 100);
    muteButton.setBounds(centerX + 160, 310, 80, 30);
    dcButton.setBounds(centerX + 250, 310, 80, 30);
}

void APTToolEditor::timerCallback()
{
    repaint();
}
