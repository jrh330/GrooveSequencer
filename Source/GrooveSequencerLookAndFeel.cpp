#include "GrooveSequencerLookAndFeel.h"

GrooveSequencerLookAndFeel::GrooveSequencerLookAndFeel()
{
    // Set up modernist font (using Helvetica as a base)
    modernistFont = juce::Font("Helvetica Neue", "Regular", 16.0f);
    setDefaultSansSerifTypefaceName("Helvetica Neue");
}

void GrooveSequencerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                     const juce::Colour& backgroundColour,
                                                     bool shouldDrawButtonAsHighlighted,
                                                     bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto baseColour = button.getToggleState() ? mint : backgroundColour;
    
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);
    
    // Draw geometric background
    if (button.getToggleState() || dynamic_cast<juce::TextButton*>(&button))
    {
        // Circular button style
        drawModernistCircle(g, bounds, baseColour);
    }
    else
    {
        // Toggle button style - rectangular with geometric accent
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 3.0f);
        drawGeometricAccent(g, bounds, baseColour.contrasting(0.5f));
    }
}

void GrooveSequencerLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.setColour(button.getToggleState() ? offWhite : black);
    
    auto bounds = button.getLocalBounds();
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

void GrooveSequencerLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPos, float minSliderPos, float maxSliderPos,
                                                 const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // Draw track
    auto trackWidth = juce::jmin(6.0f, float(width) * 0.25f);
    auto trackBounds = juce::Rectangle<float>(x + width * 0.5f - trackWidth * 0.5f,
                                            y, trackWidth, height);
    
    g.setColour(mint.withAlpha(0.3f));
    g.fillRoundedRectangle(trackBounds, trackWidth * 0.5f);
    
    // Draw value track
    auto valueTrackBounds = trackBounds.withBottom(sliderPos);
    g.setColour(mint);
    g.fillRoundedRectangle(valueTrackBounds, trackWidth * 0.5f);
    
    // Draw thumb
    auto thumbWidth = trackWidth * 2.5f;
    auto thumbBounds = juce::Rectangle<float>(thumbWidth, thumbWidth)
                          .withCentre(juce::Point<float>(trackBounds.getCentreX(), sliderPos));
    
    drawModernistCircle(g, thumbBounds, yellow);
}

void GrooveSequencerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPosProportional, float rotaryStartAngle,
                                                 float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;
    auto centre = bounds.getCentre();
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw outer circle
    drawModernistCircle(g, bounds.reduced(bounds.getWidth() * 0.1f), mint.withAlpha(0.3f));
    
    // Draw value arc
    g.setColour(mint);
    auto path = juce::Path();
    path.addArc(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f,
                rotaryStartAngle, angle, true);
    g.strokePath(path, juce::PathStrokeType(3.0f));
    
    // Draw pointer
    auto pointerLength = radius * 0.8f;
    auto pointerThickness = 3.0f;
    auto pointerPath = juce::Path();
    pointerPath.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    g.setColour(yellow);
    g.fillPath(pointerPath, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void GrooveSequencerLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                             bool isButtonDown, int buttonX, int buttonY,
                                             int buttonW, int buttonH, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    // Draw background
    g.setColour(mint.withAlpha(0.1f));
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Draw border
    g.setColour(mint);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
    
    // Draw arrow
    auto arrowBounds = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH);
    auto arrowPath = juce::Path();
    arrowPath.addTriangle(arrowBounds.getCentreX() - 3.0f, arrowBounds.getCentreY() - 2.0f,
                         arrowBounds.getCentreX() + 3.0f, arrowBounds.getCentreY() - 2.0f,
                         arrowBounds.getCentreX(), arrowBounds.getCentreY() + 2.0f);
    g.setColour(box.isEnabled() ? mint : mint.withAlpha(0.3f));
    g.fillPath(arrowPath);
}

void GrooveSequencerLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setFont(getLabelFont(label));
    g.setColour(black);
    
    auto bounds = label.getLocalBounds();
    g.drawText(label.getText(), bounds, label.getJustificationType());
}

void GrooveSequencerLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto baseColour = button.getToggleState() ? mint : offWhite;
    
    // Draw background
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Draw border
    g.setColour(mint);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
    
    // Draw text
    g.setFont(modernistFont);
    g.setColour(button.getToggleState() ? offWhite : black);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

juce::Font GrooveSequencerLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return modernistFont.withHeight(buttonHeight * 0.4f);
}

juce::Font GrooveSequencerLookAndFeel::getLabelFont(juce::Label&)
{
    return modernistFont;
}

juce::Font GrooveSequencerLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return modernistFont;
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

juce::Colour GrooveSequencerLookAndFeel::getBackgroundColour() const
{
    return offWhite;
}

juce::Colour GrooveSequencerLookAndFeel::getBackgroundAccentColour() const
{
    return red.withAlpha(0.8f);
}

juce::Colour GrooveSequencerLookAndFeel::getGridCellBorderColour() const
{
    return blue;
}

juce::Colour GrooveSequencerLookAndFeel::getGridCellStaccatoColour() const
{
    return red;
}

juce::Colour GrooveSequencerLookAndFeel::getPlayheadColour() const
{
    return blue.withAlpha(0.5f);
}

juce::Colour GrooveSequencerLookAndFeel::getGridLineColour() const
{
    return blue.withAlpha(0.3f);
} 