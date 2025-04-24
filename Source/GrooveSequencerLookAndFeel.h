#pragma once

#include <JuceHeader.h>

class GrooveSequencerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrooveSequencerLookAndFeel();
    
    juce::Colour getBackgroundColour() const;
    juce::Colour getBackgroundAccentColour() const;
    juce::Colour getGridCellBorderColour() const;
    juce::Colour getGridCellStaccatoColour() const;
    juce::Colour getPlayheadColour() const;
    juce::Colour getGridLineColour() const;
    
    // Color palette
    const juce::Colour mint      { 0xFF40B0A2 };  // Olivetti turquoise
    const juce::Colour yellow    { 0xFFFFC857 };  // Warm yellow
    const juce::Colour red       { 0xFFE94F37 };  // Bright red
    const juce::Colour blue      { 0xFF1B98E0 };  // Accent blue
    const juce::Colour offWhite  { 0xFFF9F7F7 };  // Background
    const juce::Colour black     { 0xFF1A1A1A };  // Rich black
    
    // Button styling
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown) override;
    
    // Slider styling
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override;
    
    // ComboBox styling
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     juce::ComboBox& box) override;
    
    // Label styling
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    
    // Toggle button styling
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    
    // Font
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    
private:
    // Helper methods for geometric shapes
    void drawModernistCircle(juce::Graphics& g, juce::Rectangle<float> bounds,
                            const juce::Colour& colour, float strokeWidth = 2.0f);
    
    void drawGeometricAccent(juce::Graphics& g, juce::Rectangle<float> bounds,
                            const juce::Colour& colour);
    
    // Custom font
    juce::Font modernistFont;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerLookAndFeel)
}; 