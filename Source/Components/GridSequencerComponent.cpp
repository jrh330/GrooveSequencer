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
    
    // Initialize grid from processor pattern
    const auto& pattern = processor.getPattern();
    for (const auto& note : pattern.notes) {
        int col = static_cast<int>(note.startTime / pattern.gridSize);
        int row = note.pitch - 60;  // Assuming base pitch is 60 (middle C)
        
        if (row >= 0 && row < kRows && col >= 0 && col < kCols) {
            grid[row][col].active = note.active;
            grid[row][col].velocity = note.velocity;
            grid[row][col].accent = note.accent;
            grid[row][col].isStaccato = note.isStaccato;
        }
    }
    
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
    DBG("GridSequencerComponent::paint called");
    DBG("Component size: " + juce::String(getWidth()) + "x" + juce::String(getHeight()));
    
    // Fill background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw decorative background
    drawBackground(g);
    
    // Draw grid
    drawGrid(g);
    
    // Draw playhead
    drawPlayhead(g);
}

void GridSequencerComponent::drawBackground(juce::Graphics& g) const
{
    auto& lf = getLookAndFeelAs();
    
    // Fill background
    g.fillAll(lf.getBackgroundColour());
    
    // Draw modernist elements
    g.setColour(lf.getBackgroundAccentColour().withAlpha(0.1f));
    
    // Geometric shapes
    const float width = getWidth();
    const float height = getHeight();
    
    // Draw diagonal lines
    g.drawLine(0, 0, width * 0.3f, height, 1.0f);
    g.drawLine(width * 0.7f, 0, width, height * 0.5f, 1.0f);
    
    // Draw angular shapes
    juce::Path path;
    path.startNewSubPath(width * 0.75f, height * 0.2f);
    path.lineTo(width * 0.85f, height * 0.3f);
    path.lineTo(width * 0.8f, height * 0.7f);
    path.lineTo(width * 0.7f, height * 0.6f);
    path.closeSubPath();
    g.strokePath(path, juce::PathStrokeType(1.0f));
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
    auto bounds = getCellBounds(row, col).reduced(kCellPadding);
    
    // Draw cell background
    g.setColour(cell.active ? 
        lf.getBackgroundAccentColour().withAlpha(0.8f) : // Active cells
        lf.getBackgroundColour().contrasting(0.1f));     // Inactive cells
    g.fillRect(bounds);
    
    // Draw cell border - orange if this is the current step and cell is active
    bool isCurrentStep = (col == currentStep && processor.isPlaying());
    g.setColour(isCurrentStep && cell.active ? 
                juce::Colours::orange : 
                cell.getRingColour(lf).withAlpha(0.9f));
    g.drawRect(bounds, cell.active ? kGridLineThicknessMajor : kGridLineThicknessMinor);
    
    // Highlight current step with a subtle background tint
    if (isCurrentStep) {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRect(bounds);
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

void GridSequencerComponent::handleCellInteraction(int row, int col, const juce::ModifierKeys& mods, float)
{
    if (!isPositionValid(row, col) || processor.isPlaying())
        return;

    auto& cell = grid[static_cast<size_t>(row)][static_cast<size_t>(col)];
    
    // Toggle cell state
    if (mods.isShiftDown()) {
        cell.isStaccato = !cell.isStaccato;
    } else if (mods.isAltDown()) {
        cell.accent = (cell.accent + 1) % 3;  // Cycle through accent levels
    } else {
        cell.active = !cell.active;
    }
    
    // Update processor pattern - use global velocity from processor
    processor.updateGridCell(row, col, cell.active, processor.getVelocityScale() * 127.0f, cell.accent, cell.isStaccato);
    repaint();
}

void GridSequencerComponent::timerCallback()
{
    const juce::ScopedLock sl(processor.getCallbackLock());
    if (processor.isPlaying()) {
        currentStep = processor.getCurrentStep();
        repaint();
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
    // Update grid cell sizes
    repaint();
}

void GridSequencerComponent::updateGridSize(int newSize)
{
    if (newSize != gridSize && newSize > 0)
    {
        gridSize = newSize;
        grid.resize(kRows);
        for (auto& row : grid)
        {
            row.resize(gridSize);
        }
        repaint();
    }
}

void GridSequencerComponent::setBasePitch(int midiNote)
{
    if (midiNote >= 0 && midiNote < 128)
    {
        basePitch = midiNote;
        // Update all cells in the grid to use the new base pitch
        for (int row = 0; row < kRows; ++row)
        {
            for (int col = 0; col < gridSize; ++col)
            {
                if (grid[row][col].active)
                {
                    processor.updateGridCell(row, col, true, grid[row][col].velocity / 127.0f,
                                          grid[row][col].accent, grid[row][col].isStaccato);
                }
            }
        }
    }
} 