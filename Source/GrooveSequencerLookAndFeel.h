#pragma once

#include <JuceHeader.h>

class GrooveSequencerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Font style enumeration for consistent typography
    enum class FontStyle {
        Regular,
        Light,
        Medium,
        Bold,
        Title,
        Heading,
        Body,
        Small
    };
    
    GrooveSequencerLookAndFeel();
    ~GrooveSequencerLookAndFeel() override = default;
    
    // Button drawing
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics&, juce::TextButton&,
                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    // Slider drawing
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override;
    
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    // ComboBox drawing
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     juce::ComboBox& box) override;
    
    void drawLabel(juce::Graphics&, juce::Label&) override;
    
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
    
    // Popup menu
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted,
                          bool isTicked, bool hasSubMenu, const juce::String& text,
                          const juce::String& shortcutKeyText,
                          const juce::Drawable* icon,
                          const juce::Colour* textColour) override;
                          
    void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;
    
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int x, int y,
                      int width, int height, bool isScrollbarVertical,
                      int thumbStartPosition, int thumbSize,
                      bool isMouseOver, bool isMouseDown) override;
                      
    void drawTooltip(juce::Graphics&, const juce::String& text, int width, int height) override;
    
    int getPopupMenuBorderSize() override { return 1; }
    int getScrollbarButtonSize(juce::ScrollBar&) override { return 20; }
    
    // Enhanced font methods
    juce::Font getFont(FontStyle style) const;
    float getFontHeight(FontStyle style) const;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTooltipFont();
    
    // Modernist design helpers
    void drawModernistCircle(juce::Graphics&, juce::Rectangle<float> bounds,
                            const juce::Colour& colour, float strokeWidth = 2.0f);
    void drawGeometricAccent(juce::Graphics&, juce::Rectangle<float> bounds,
                            const juce::Colour& colour);
    
    // Olivetti 82 inspired color palette
    const juce::Colour background { juce::Colour(245, 243, 238) };  // Cream
    const juce::Colour primary    { juce::Colour(0, 0, 0) };       // Black
    const juce::Colour secondary  { juce::Colour(0, 150, 70) };    // Green
    const juce::Colour accent1    { juce::Colour(240, 80, 35) };   // Red/Orange
    const juce::Colour accent2    { juce::Colour(255, 200, 40) };  // Yellow
    const juce::Colour accent3    { juce::Colour(65, 145, 210) };  // Light Blue
    
    // Color getters
    juce::Colour getBackgroundColor() const { return backgroundColour; }
    juce::Colour getBackgroundAccentColour() const { return accent2; }
    juce::Colour getGridCellBorderColour() const { return secondary; }
    juce::Colour getGridCellStaccatoColour() const { return accent1; }
    juce::Colour getPlayheadColour() const { return accent3.withAlpha(0.5f); }
    juce::Colour getGridLineColour() const { return accent3.withAlpha(0.3f); }
    
    juce::Colour getTextColor() const { return textColour; }
    juce::Colour getHighlightColor() const { return highlightColour; }
    
private:
    // Font family definitions
    juce::String getPrimaryFontName() const;
    juce::String getFallbackFontName() const;
    
    // Font size constants
    const float BASE_FONT_SIZE = 14.0f;
    const float TITLE_FONT_SIZE = BASE_FONT_SIZE * 1.8f;    // 25.2px
    const float HEADING_FONT_SIZE = BASE_FONT_SIZE * 1.4f;  // 19.6px
    const float BODY_FONT_SIZE = BASE_FONT_SIZE;            // 14.0px
    const float SMALL_FONT_SIZE = BASE_FONT_SIZE * 0.85f;   // 11.9px
    
    // Font instances for different styles
    juce::Font regularFont;
    juce::Font lightFont;
    juce::Font mediumFont;
    juce::Font boldFont;
    
    juce::Colour backgroundColour;
    juce::Colour textColour;
    juce::Colour highlightColour;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerLookAndFeel)
}; 