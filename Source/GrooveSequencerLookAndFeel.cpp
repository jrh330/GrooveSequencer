#include "GrooveSequencerLookAndFeel.h"

GrooveSequencerLookAndFeel::GrooveSequencerLookAndFeel()
{
    // Initialize fonts with primary and fallback options
    regularFont = juce::Font(getPrimaryFontName(), BASE_FONT_SIZE, juce::Font::plain);
    if (!regularFont.isAvailable())
        regularFont = juce::Font(getFallbackFontName(), BASE_FONT_SIZE, juce::Font::plain);
        
    lightFont = regularFont.withStyle(juce::Font::plain);
    lightFont.setExtraKerningFactor(0.05f);
    
    mediumFont = regularFont.withStyle(juce::Font::plain);
    mediumFont.setExtraKerningFactor(-0.02f);
    
    boldFont = regularFont.withStyle(juce::Font::bold);
    boldFont.setExtraKerningFactor(-0.05f);

    // Set colors using our Olivetti-inspired palette
    setColour(juce::ResizableWindow::backgroundColourId, background);
    setColour(juce::Slider::thumbColourId, primary);
    setColour(juce::Slider::trackColourId, secondary);
    setColour(juce::Slider::backgroundColourId, background.darker(0.1f));
    setColour(juce::TextButton::buttonColourId, accent1);
    setColour(juce::TextButton::buttonOnColourId, accent2);
    setColour(juce::TextButton::textColourOffId, background);
    setColour(juce::TextButton::textColourOnId, primary);
    setColour(juce::Label::textColourId, primary);
    setColour(juce::ComboBox::backgroundColourId, background);
    setColour(juce::ComboBox::textColourId, primary);
    setColour(juce::ComboBox::outlineColourId, secondary);
    
    // New color settings for added components
    setColour(juce::PopupMenu::backgroundColourId, background);
    setColour(juce::PopupMenu::textColourId, primary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accent2);
    setColour(juce::PopupMenu::highlightedTextColourId, primary);
    setColour(juce::ScrollBar::backgroundColourId, background.darker(0.1f));
    setColour(juce::ScrollBar::thumbColourId, secondary);
    setColour(juce::ScrollBar::trackColourId, background);
    setColour(juce::TooltipWindow::backgroundColourId, primary.withAlpha(0.9f));
    setColour(juce::TooltipWindow::textColourId, background);
}

juce::String GrooveSequencerLookAndFeel::getPrimaryFontName() const
{
    // Try system fonts in order of preference
    const juce::StringArray fontNames = {
        "Helvetica Neue",
        "SF Pro Text",
        "Inter",
        "Roboto",
        "Arial"
    };
    
    for (const auto& fontName : fontNames)
        if (juce::Font(fontName, BASE_FONT_SIZE, juce::Font::plain).isAvailable())
            return fontName;
            
    return "Helvetica Neue"; // Default to Helvetica Neue
}

juce::String GrooveSequencerLookAndFeel::getFallbackFontName() const
{
    return juce::Font::getDefaultSansSerifFontName();
}

juce::Font GrooveSequencerLookAndFeel::getFont(FontStyle style) const
{
    switch (style)
    {
        case FontStyle::Light:
            return lightFont;
        case FontStyle::Medium:
            return mediumFont;
        case FontStyle::Bold:
            return boldFont;
        case FontStyle::Title:
            return boldFont.withHeight(TITLE_FONT_SIZE);
        case FontStyle::Heading:
            return mediumFont.withHeight(HEADING_FONT_SIZE);
        case FontStyle::Body:
            return regularFont;
        case FontStyle::Small:
            return regularFont.withHeight(SMALL_FONT_SIZE);
        default:
            return regularFont;
    }
}

float GrooveSequencerLookAndFeel::getFontHeight(FontStyle style) const
{
    switch (style)
    {
        case FontStyle::Title:   return TITLE_FONT_SIZE;
        case FontStyle::Heading: return HEADING_FONT_SIZE;
        case FontStyle::Body:    return BODY_FONT_SIZE;
        case FontStyle::Small:   return SMALL_FONT_SIZE;
        default:                 return BASE_FONT_SIZE;
    }
}

void GrooveSequencerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                     const juce::Colour& backgroundColour,
                                                     bool shouldDrawButtonAsHighlighted,
                                                     bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto baseColour = button.getToggleState() ? accent2 : accent1;
    
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);
    
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Add geometric accent
    g.setColour(primary);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}

void GrooveSequencerLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    g.setFont(getFont(FontStyle::Medium).withHeight(button.getHeight() * 0.4f));
    g.setColour(button.getToggleState() ? primary : background);
    
    auto bounds = button.getLocalBounds();
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

void GrooveSequencerLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPos, float minSliderPos, float maxSliderPos,
                                                 const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    auto trackBounds = bounds.reduced(2.0f);
    
    // Draw track background
    g.setColour(background.darker(0.1f));
    g.fillRoundedRectangle(trackBounds, 3.0f);
    
    // Draw filled portion
    g.setColour(secondary);
    auto filledBounds = trackBounds.withWidth(sliderPos - x);
    g.fillRoundedRectangle(filledBounds, 3.0f);
    
    // Draw thumb
    g.setColour(primary);
    auto thumbBounds = juce::Rectangle<float>(sliderPos - 5.0f, bounds.getY(), 10.0f, bounds.getHeight());
    g.fillRoundedRectangle(thumbBounds, 2.0f);
}

void GrooveSequencerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPosProportional, float rotaryStartAngle,
                                                 float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineWidth = radius * 0.1f;
    auto arcRadius = radius - lineWidth * 2.0f;
    
    bounds.reduce(lineWidth, lineWidth);
    
    // Draw background circle
    g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    g.fillEllipse(bounds);
    
    // Draw arc
    g.setColour(slider.findColour(juce::Slider::trackColourId));
    juce::Path arc;
    arc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                      0.0f, rotaryStartAngle, toAngle, true);
    g.strokePath(arc, juce::PathStrokeType(lineWidth));
    
    // Draw thumb
    juce::Path thumb;
    auto thumbWidth = lineWidth * 2.0f;
    thumb.addRectangle(-thumbWidth / 2, -radius + lineWidth * 2, thumbWidth, lineWidth * 4);
    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    g.fillPath(thumb, juce::AffineTransform::rotation(toAngle).translated(bounds.getCentreX(), bounds.getCentreY()));
}

void GrooveSequencerLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                             bool isButtonDown, int buttonX, int buttonY,
                                             int buttonW, int buttonH, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
    
    // Draw arrow
    auto arrowBounds = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH);
    juce::Path arrow;
    arrow.startNewSubPath(arrowBounds.getX() + arrowBounds.getWidth() * 0.3f, arrowBounds.getCentreY() - 2.0f);
    arrow.lineTo(arrowBounds.getCentreX(), arrowBounds.getCentreY() + 2.0f);
    arrow.lineTo(arrowBounds.getX() + arrowBounds.getWidth() * 0.7f, arrowBounds.getCentreY() - 2.0f);
    
    g.setColour(box.findColour(juce::ComboBox::textColourId));
    g.strokePath(arrow, juce::PathStrokeType(1.0f));
}

void GrooveSequencerLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setFont(getLabelFont(label));
    g.setColour(primary);
    
    auto bounds = label.getLocalBounds();
    g.drawText(label.getText(), bounds, label.getJustificationType());
}

void GrooveSequencerLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto baseColour = button.getToggleState() ? accent2 : accent1;
    
    // Draw background
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Draw border
    g.setColour(primary);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
    
    // Draw text with medium weight font
    g.setFont(getFont(FontStyle::Medium));
    g.setColour(button.getToggleState() ? primary : background);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

juce::Font GrooveSequencerLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return getFont(FontStyle::Medium).withHeight(buttonHeight * 0.4f);
}

juce::Font GrooveSequencerLookAndFeel::getLabelFont(juce::Label& label)
{
    // Use different styles based on label role (if needed)
    if (label.getProperties().contains("FontStyle"))
    {
        const auto styleString = label.getProperties()["FontStyle"].toString();
        if (styleString == "Title")   return getFont(FontStyle::Title);
        if (styleString == "Heading") return getFont(FontStyle::Heading);
        if (styleString == "Small")   return getFont(FontStyle::Small);
    }
    return getFont(FontStyle::Body);
}

juce::Font GrooveSequencerLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return getFont(FontStyle::Regular);
}

void GrooveSequencerLookAndFeel::drawModernistCircle(juce::Graphics& g,
                                                    juce::Rectangle<float> bounds,
                                                    const juce::Colour& colour,
                                                    float strokeWidth)
{
    auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    auto radius = diameter * 0.5f;
    auto centre = bounds.getCentre();
    
    // Fill
    g.setColour(colour);
    g.fillEllipse(centre.x - radius, centre.y - radius, diameter, diameter);
    
    // Stroke
    g.setColour(colour.contrasting(0.2f));
    g.drawEllipse(centre.x - radius, centre.y - radius, diameter, diameter, strokeWidth);
}

void GrooveSequencerLookAndFeel::drawGeometricAccent(juce::Graphics& g,
                                                    juce::Rectangle<float> bounds,
                                                    const juce::Colour& colour)
{
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.2f;
    auto path = juce::Path();
    
    // Draw a small geometric accent (triangle or line)
    path.addTriangle(bounds.getRight() - size * 2.0f, bounds.getCentreY(),
                    bounds.getRight() - size, bounds.getCentreY() - size * 0.5f,
                    bounds.getRight() - size, bounds.getCentreY() + size * 0.5f);
    
    g.setColour(colour);
    g.fillPath(path);
}

void GrooveSequencerLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                                  bool isSeparator, bool isActive, bool isHighlighted,
                                                  bool isTicked, bool hasSubMenu, const juce::String& text,
                                                  const juce::String& shortcutKeyText,
                                                  const juce::Drawable* icon, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        auto r = area.reduced(5, 2);
        g.setColour(primary.withAlpha(0.3f));
        g.drawLine(r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 1.0f);
    }
    else
    {
        auto r = area.reduced(1);
        
        if (isHighlighted && isActive)
        {
            g.setColour(accent2);
            g.fillRect(r);
            g.setColour(primary);
        }
        else
        {
            g.setColour(textColour != nullptr ? *textColour : findColour(juce::PopupMenu::textColourId));
        }
        
        r.reduce(juce::jmin(5, area.getWidth() / 20), 0);
        
        auto font = getFont(FontStyle::Regular);
        auto maxFontHeight = r.getHeight() / 1.3f;
        
        if (font.getHeight() > maxFontHeight)
            font.setHeight(maxFontHeight);
            
        g.setFont(font);
        
        if (isTicked)
        {
            auto checkboxSize = font.getHeight();
            auto checkboxArea = r.removeFromLeft(checkboxSize).reduced(3);
            drawModernistCircle(g, checkboxArea.toFloat(), accent1, 1.0f);
        }
        
        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();
            auto x = static_cast<float>(r.removeFromRight((int) arrowH).getX());
            auto halfH = static_cast<float>(r.getCentreY());
            
            juce::Path path;
            path.startNewSubPath(x, halfH - arrowH * 0.5f);
            path.lineTo(x + arrowH * 0.6f, halfH);
            path.lineTo(x, halfH + arrowH * 0.5f);
            
            g.strokePath(path, juce::PathStrokeType(2.0f));
        }
        
        r.removeFromRight(3);
        g.drawFittedText(text, r, juce::Justification::centredLeft, 1);
        
        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = getFont(FontStyle::Small);
            g.setFont(f2);
            g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
        }
    }
}

void GrooveSequencerLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto background = findColour(juce::PopupMenu::backgroundColourId);
    g.fillAll(background);
    
    g.setColour(primary.withAlpha(0.2f));
    g.drawRect(0, 0, width, height, 1);
    
    // Add subtle gradient
    juce::ColourGradient gradient(background.brighter(0.02f), 0.0f, 0.0f,
                                 background.darker(0.02f), 0.0f, static_cast<float>(height), false);
    g.setGradientFill(gradient);
    g.fillAll();
}

void GrooveSequencerLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                                              int x, int y, int width, int height,
                                              bool isScrollbarVertical, int thumbStartPosition,
                                              int thumbSize, bool isMouseOver, bool isMouseDown)
{
    auto thumbBounds = juce::Rectangle<int>(x + (isScrollbarVertical ? 0 : thumbStartPosition),
                                          y + (isScrollbarVertical ? thumbStartPosition : 0),
                                          isScrollbarVertical ? width : thumbSize,
                                          isScrollbarVertical ? thumbSize : height);
    
    // Draw track
    g.setColour(findColour(juce::ScrollBar::trackColourId));
    g.fillRoundedRectangle(x, y, width, height, 3.0f);
    
    // Draw thumb
    auto thumbCol = findColour(juce::ScrollBar::thumbColourId)
                    .withMultipliedAlpha(isMouseOver ? 1.1f : 0.9f);
    g.setColour(thumbCol);
    g.fillRoundedRectangle(thumbBounds.toFloat(), 3.0f);
    
    // Add geometric accent
    if (isMouseOver || isMouseDown)
    {
        g.setColour(primary.withAlpha(0.5f));
        auto accentBounds = thumbBounds.toFloat().reduced(2.0f);
        if (isScrollbarVertical)
        {
            g.drawLine(accentBounds.getCentreX(), accentBounds.getY(),
                      accentBounds.getCentreX(), accentBounds.getBottom(), 1.0f);
        }
        else
        {
            g.drawLine(accentBounds.getX(), accentBounds.getCentreY(),
                      accentBounds.getRight(), accentBounds.getCentreY(), 1.0f);
        }
    }
}

void GrooveSequencerLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto bounds = juce::Rectangle<int>(width, height).toFloat();
    auto cornerSize = 3.0f;
    
    // Draw background
    g.setColour(findColour(juce::TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Draw border
    g.setColour(primary.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    
    // Draw text
    g.setColour(findColour(juce::TooltipWindow::textColourId));
    g.setFont(getTooltipFont());
    g.drawText(text, bounds.reduced(4), juce::Justification::centred, true);
}

juce::Font GrooveSequencerLookAndFeel::getPopupMenuFont()
{
    return getFont(FontStyle::Regular);
}

juce::Font GrooveSequencerLookAndFeel::getTooltipFont()
{
    return getFont(FontStyle::Small);
} 