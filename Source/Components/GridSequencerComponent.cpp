#include "GridSequencerComponent.h"
#include "../PluginProcessor.h"
#include "../GrooveSequencerLookAndFeel.h"

namespace {
    constexpr float kCellPadding = 2.0f;
    constexpr float kPlayheadWidth = 2.0f;
    constexpr float kGridLineThicknessMajor = 2.0f;
    constexpr float kGridLineThicknessMinor = 1.0f;
    constexpr float kVelocityIndicatorWidthRatio = 0.2f;
    constexpr float kAccentDotSize = 4.0f;
    constexpr float kAccentDotSpacing = 8.0f;
}

void GridCell::updateState(bool newActive, float newVelocity, int newAccent, bool newStaccato)
{
    active = newActive;
    velocity = juce::jlimit(0.0f, 127.0f, newVelocity);
    accent = juce::jlimit(0, 2, newAccent);
    isStaccato = newStaccato;
}

juce::Colour GridCell::getBaseColour(const GrooveSequencerLookAndFeel& lf) const
{
    if (!active) return lf.getBackgroundColour();
    
    auto colour = accent > 0 
        ? lf.getBackgroundAccentColour().withBrightness(0.7f + accent * 0.15f)
        : lf.getGridCellBorderColour();
    
    return colour.withAlpha(velocity / 127.0f);
}

juce::Colour GridCell::getRingColour(const GrooveSequencerLookAndFeel& lf) const
{
    if (!active) return lf.getGridCellBorderColour().withAlpha(0.3f);
    return isStaccato ? lf.getGridCellStaccatoColour() : getBaseColour(lf).withAlpha(0.5f);
}

GridSequencerComponent::GridSequencerComponent(GrooveSequencerAudioProcessor& p)
    : processor(p)
    , grid(kRows, std::vector<GridCell>(kCols))
{
    setOpaque(true);
    startTimerHz(60);
}

GridSequencerComponent::~GridSequencerComponent()
{
    stopTimer();
}

GrooveSequencerLookAndFeel& GridSequencerComponent::getLookAndFeelAs() const
{
    return dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
}

void GridSequencerComponent::paint(juce::Graphics& g)
{
    drawBackground(g);
    drawGrid(g);
    drawPlayhead(g);
}

void GridSequencerComponent::drawBackground(juce::Graphics& g) const
{
    auto& lf = getLookAndFeelAs();
    
    // Fill background
    g.fillAll(lf.getBackgroundColour());
    
    // Draw modernist elements
    g.setColour(lf.getBackgroundAccentColour().withAlpha(0.1f));
    
    // Large circle
    const float centerX = getWidth() * 0.75f;
    const float centerY = getHeight() * 0.5f;
    const float radius = getHeight() * 0.8f;
    g.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2, 1.0f);
    
    // Diagonal lines
    g.drawLine(0, 0, getWidth() * 0.3f, getHeight(), 1.0f);
    g.drawLine(getWidth() * 0.7f, 0, getWidth(), getHeight() * 0.5f, 1.0f);
}

void GridSequencerComponent::drawGrid(juce::Graphics& g) const
{
    auto& lf = getLookAndFeelAs();
    const float cellWidth = getCellWidth();
    const float cellHeight = getCellHeight();
    
    // Draw grid lines
    for (int col = 0; col <= kCols; ++col) {
        const float x = col * cellWidth;
        const float thickness = (col % 4 == 0) ? kGridLineThicknessMajor : kGridLineThicknessMinor;
        g.setColour(lf.getGridLineColour().withAlpha(col % 4 == 0 ? 0.8f : 0.4f));
        g.drawLine(x, 0, x, getHeight(), thickness);
    }
    
    for (int row = 0; row <= kRows; ++row) {
        const float y = row * cellHeight;
        g.setColour(lf.getGridLineColour().withAlpha(0.4f));
        g.drawLine(0, y, getWidth(), y, kGridLineThicknessMinor);
    }
    
    // Draw cells
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            drawCell(g, row, col);
        }
    }
}

void GridSequencerComponent::drawCell(juce::Graphics& g, int row, int col) const
{
    if (!isPositionValid(row, col)) return;
        
    const auto& cell = grid[static_cast<size_t>(row)][static_cast<size_t>(col)];
    auto& lf = getLookAndFeelAs();
    auto bounds = getCellBounds(row, col);
    
    // Draw cell background
    g.setColour(cell.getBaseColour(lf));
    g.fillEllipse(bounds);
    
    if (cell.active) {
        // Draw ring
        g.setColour(cell.getRingColour(lf));
        g.drawEllipse(bounds, kGridLineThicknessMajor);
        
        // Draw velocity indicator
        if (!cell.isStaccato) {
            const float velocityHeight = (cell.velocity / 127.0f) * bounds.getHeight() * 0.6f;
            const float indicatorWidth = bounds.getWidth() * kVelocityIndicatorWidthRatio;
            g.fillRect(bounds.getCentreX() - indicatorWidth * 0.5f,
                      bounds.getCentreY() + bounds.getHeight() * 0.1f - velocityHeight,
                      indicatorWidth,
                      velocityHeight);
        }
        
        // Draw accent dots
        if (cell.accent > 0) {
            const float startX = bounds.getCentreX() - ((cell.accent - 1) * kAccentDotSpacing * 0.5f);
            const float y = bounds.getY() + bounds.getHeight() * 0.2f;
            
            for (int i = 0; i < cell.accent; ++i) {
                g.fillEllipse(startX + (i * kAccentDotSpacing) - kAccentDotSize * 0.5f,
                             y - kAccentDotSize * 0.5f,
                             kAccentDotSize, kAccentDotSize);
            }
        }
    } else {
        g.setColour(cell.getRingColour(lf));
        g.drawEllipse(bounds, kGridLineThicknessMinor);
    }
}

void GridSequencerComponent::drawPlayhead(juce::Graphics& g) const
{
    const juce::ScopedLock sl(processor.getCallbackLock());
    if (processor.isPlaying()) {
        auto& lf = getLookAndFeelAs();
        g.setColour(lf.getPlayheadColour());
        const float playheadX = currentStep * getCellWidth();
        g.drawLine(playheadX, 0, playheadX, getHeight(), kPlayheadWidth);
    }
}

void GridSequencerComponent::mouseDown(const juce::MouseEvent& event)
{
    int row, col;
    if (getCellFromPoint(event.position.toInt(), row, col)) {
        dragState = { true, row, col, event.position.y };
        handleCellInteraction(row, col, event.mods);
    }
}

void GridSequencerComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (!dragState.isDragging) return;
    
    int row, col;
    if (getCellFromPoint(event.position.toInt(), row, col)) {
        const float dragDelta = (dragState.startY - event.position.y) / static_cast<float>(getHeight());
        
        if (row != dragState.startRow || col != dragState.startCol) {
            handleCellInteraction(row, col, event.mods);
            dragState.startRow = row;
            dragState.startCol = col;
        }
        
        handleCellInteraction(row, col, event.mods, dragDelta);
    }
}

void GridSequencerComponent::mouseUp(const juce::MouseEvent&)
{
    dragState = {};
}

void GridSequencerComponent::handleCellInteraction(int row, int col, const juce::ModifierKeys& mods, float dragDelta)
{
    if (!isPositionValid(row, col)) {
        juce::Logger::writeToLog("Invalid grid position: " + juce::String(row) + ", " + juce::String(col));
        return;
    }
    
    auto& cell = grid[static_cast<size_t>(row)][static_cast<size_t>(col)];
    bool shouldUpdate = false;
    
    if (dragDelta != 0.0f && cell.active) {
        // Update velocity based on drag
        float newVelocity = juce::jlimit(1.0f, 127.0f, cell.velocity + dragDelta * 64.0f);
        if (newVelocity != cell.velocity) {
            cell.velocity = newVelocity;
            shouldUpdate = true;
        }
    } else if (mods.isRightButtonDown() && cell.active) {
        // Cycle accent
        cell.accent = (cell.accent + 1) % 3;
        shouldUpdate = true;
    } else if (mods.isAltDown() && cell.active) {
        // Toggle staccato
        cell.isStaccato = !cell.isStaccato;
        shouldUpdate = true;
    } else {
        // Toggle active state
        bool wasActive = cell.active;
        cell.updateState(!cell.active, 
                        cell.active ? cell.velocity : 100.0f,
                        cell.active ? cell.accent : 0,
                        cell.active ? cell.isStaccato : false);
        shouldUpdate = (wasActive != cell.active);
    }
    
    if (shouldUpdate) {
        const juce::ScopedLock sl(processor.getCallbackLock());
        processor.updateGridCell(row, col, cell.active, cell.velocity, cell.accent, cell.isStaccato);
        repaint();
    }
}

void GridSequencerComponent::timerCallback()
{
    const juce::ScopedLock sl(processor.getCallbackLock());
    if (processor.isPlaying()) {
        const int newStep = processor.getCurrentStep();
        if (newStep != currentStep && newStep >= 0 && newStep < kCols) {
            currentStep = newStep;
            repaint();
        }
    }
}

juce::Rectangle<float> GridSequencerComponent::getCellBounds(int row, int col) const
{
    const float cellWidth = getCellWidth();
    const float cellHeight = getCellHeight();
    return { col * cellWidth + kCellPadding, 
             row * cellHeight + kCellPadding, 
             cellWidth - 2 * kCellPadding, 
             cellHeight - 2 * kCellPadding };
}

bool GridSequencerComponent::getCellFromPoint(juce::Point<int> point, int& row, int& col) const
{
    if (point.x < 0 || point.y < 0 || point.x >= getWidth() || point.y >= getHeight()) {
        return false;
    }

    col = static_cast<int>(point.x * static_cast<float>(kCols) / static_cast<float>(getWidth()));
    row = static_cast<int>(point.y * static_cast<float>(kRows) / static_cast<float>(getHeight()));
    return isPositionValid(row, col);
}

bool GridSequencerComponent::isPositionValid(int row, int col) const
{
    return row >= 0 && row < kRows && col >= 0 && col < kCols;
}

void GridSequencerComponent::resized()
{
    repaint();
} 