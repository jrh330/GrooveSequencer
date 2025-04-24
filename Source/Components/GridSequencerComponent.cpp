#include "GridSequencerComponent.h"
#include "../PluginProcessor.h"
#include "../GrooveSequencerLookAndFeel.h"

GridSequencerComponent::GridSequencerComponent(GrooveSequencerAudioProcessor& p)
    : processor(p)
    , grid(kRows, std::vector<GridCell>(kCols))
{
    // Initialize grid cells
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
            grid[row][col].updateColour(lf);
        }
    }
    
    // Start timer for playhead updates (60 fps)
    startTimerHz(60);
    
    setOpaque(true);
}

GridSequencerComponent::~GridSequencerComponent()
{
    stopTimer();
}

void GridSequencerComponent::paint(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getBackgroundColor());
    
    // Draw modernist background elements
    drawModernistBackground(g);
    
    // Draw grid lines
    drawGridLines(g);
    
    // Draw cells
    const int cellWidth = getWidth() / kCols;
    const int cellHeight = getHeight() / kRows;
    
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            juce::Rectangle<int> cellBounds(col * cellWidth + 2,
                                          row * cellHeight + 2,
                                          cellWidth - 4,
                                          cellHeight - 4);
            drawCell(g, col, row, cellBounds);
        }
    }
    
    // Draw playhead if playing
    if (processor.isPlaying()) {
        g.setColour(lf.getPlayheadColor());
        const int playheadX = processor.getCurrentStep() * cellWidth;
        g.drawLine(playheadX, 0, playheadX, getHeight(), 2.0f);
    }
}

void GridSequencerComponent::resized()
{
    // The grid automatically adjusts based on component size
}

void GridSequencerComponent::mouseDown(const juce::MouseEvent& event)
{
    int row, col;
    if (getCellFromPoint(event.position.toInt(), row, col))
    {
        isDragging = true;
        lastRow = row;
        lastCol = col;
        handleCellClick(row, col, event.mods);
    }
}

void GridSequencerComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (!isDragging) return;
    
    int row, col;
    if (getCellFromPoint(event.position.toInt(), row, col))
    {
        if (row != lastRow || col != lastCol)
        {
            handleCellClick(row, col, event.mods);
            lastRow = row;
            lastCol = col;
        }
        else
        {
            // Update velocity based on vertical drag
            float dragDelta = event.position.y - event.mouseDownPosition.y;
            updateCellVelocity(row, col, dragDelta);
        }
    }
}

void GridSequencerComponent::mouseUp(const juce::MouseEvent&)
{
    isDragging = false;
    lastRow = -1;
    lastCol = -1;
}

void GridSequencerComponent::timerCallback()
{
    // Only repaint if the playhead needs updating
    if (processor.isPlaying())
        repaint();
}

void GridSequencerComponent::handleCellClick(int row, int col, const juce::ModifierKeys& mods)
{
    if (row < 0 || row >= kRows || col < 0 || col >= kCols)
        return;
        
    auto& cell = grid[row][col];
    
    if (mods.isShiftDown())
    {
        // Toggle staccato
        cell.isStaccato = !cell.isStaccato;
    }
    else if (mods.isAltDown())
    {
        // Cycle through accent levels
        updateCellAccent(row, col);
    }
    else
    {
        // Toggle active state
        cell.active = !cell.active;
    }
    
    // Update the processor's pattern
    processor.updateGridCell(row, col, cell.active, cell.velocity, cell.accent, cell.isStaccato);
    repaint();
}

void GridSequencerComponent::updateCellVelocity(int row, int col, float dragDelta)
{
    if (row < 0 || row >= kRows || col < 0 || col >= kCols)
        return;
        
    auto& cell = grid[row][col];
    if (!cell.active)
        return;
        
    // Update velocity based on drag amount (negative drag = higher velocity)
    int newVelocity = cell.velocity - static_cast<int>(dragDelta * 0.5f);
    cell.velocity = juce::jlimit(1, 127, newVelocity);
    
    // Update the processor's pattern
    processor.updateGridCell(row, col, cell.active, cell.velocity, cell.accent, cell.isStaccato);
    repaint();
}

void GridSequencerComponent::updateCellAccent(int row, int col)
{
    if (row < 0 || row >= kRows || col < 0 || col >= kCols)
        return;
        
    auto& cell = grid[row][col];
    if (!cell.active)
        return;
        
    // Cycle through accent levels: 0 -> 1 -> 2 -> 0
    cell.accent = (cell.accent + 1) % 3;
    
    // Update the processor's pattern
    processor.updateGridCell(row, col, cell.active, cell.velocity, cell.accent, cell.isStaccato);
    repaint();
}

void GridSequencerComponent::drawModernistBackground(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    
    // Draw geometric shapes in background
    g.setColour(lf.getGeometricShapeColor().withAlpha(0.1f));
    
    // Draw large circle
    float centerX = getWidth() * 0.75f;
    float centerY = getHeight() * 0.5f;
    float radius = getHeight() * 0.8f;
    g.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2, 1.0f);
    
    // Draw diagonal lines
    g.drawLine(0, 0, getWidth() * 0.3f, getHeight(), 1.0f);
    g.drawLine(getWidth() * 0.7f, 0, getWidth(), getHeight() * 0.5f, 1.0f);
}

void GridSequencerComponent::drawGridLines(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    const float cellWidth = getWidth() / static_cast<float>(kCols);
    const float cellHeight = getHeight() / static_cast<float>(kRows);
    
    // Draw vertical lines
    for (int col = 0; col <= kCols; ++col) {
        const float x = col * cellWidth;
        const float thickness = (col % 4 == 0) ? 2.0f : 1.0f;
        g.setColour(lf.getGridLineColor().withAlpha(col % 4 == 0 ? 0.8f : 0.4f));
        g.drawLine(x, 0, x, getHeight(), thickness);
    }
    
    // Draw horizontal lines
    for (int row = 0; row <= kRows; ++row) {
        const float y = row * cellHeight;
        g.setColour(lf.getGridLineColor().withAlpha(0.4f));
        g.drawLine(0, y, getWidth(), y, 1.0f);
    }
}

juce::Point<int> GridSequencerComponent::getCellPosition(int row, int col) const
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    const GridCell& cell = grid[row][col];
    auto bounds = getCellBounds(row, col).toFloat().reduced(2.0f);
    
    // Draw cell background
    if (cell.active) {
        // Draw main circle
        g.setColour(cell.baseColour);
        g.fillEllipse(bounds);
        
        // Draw ring
        g.setColour(cell.ringColour);
        g.drawEllipse(bounds, 2.0f);
        
        // Draw velocity indicator
        if (!cell.isStaccato) {
            float velocityHeight = (cell.velocity / 127.0f) * bounds.getHeight() * 0.6f;
            float indicatorWidth = bounds.getWidth() * 0.2f;
            g.fillRect(bounds.getCentreX() - indicatorWidth * 0.5f,
                      bounds.getCentreY() + bounds.getHeight() * 0.1f - velocityHeight,
                      indicatorWidth,
                      velocityHeight);
        }
        
        // Draw accent dots
        if (cell.accent > 0) {
            float dotSize = 4.0f;
            float spacing = dotSize * 2.0f;
            float startX = bounds.getCentreX() - ((cell.accent - 1) * spacing * 0.5f);
            float y = bounds.getY() + bounds.getHeight() * 0.2f;
            
            for (int i = 0; i < cell.accent; ++i) {
                g.fillEllipse(startX + (i * spacing) - dotSize * 0.5f,
                             y - dotSize * 0.5f,
                             dotSize, dotSize);
            }
        }
    } else {
        // Inactive cell
        g.setColour(cell.baseColour);
        g.fillEllipse(bounds);
        g.setColour(cell.ringColour);
        g.drawEllipse(bounds, 1.0f);
    }
}

void GridSequencerComponent::drawCell(juce::Graphics& g, int row, int col, const GridCell& cell)
{
    auto bounds = getCellBounds(row, col);
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    
    // Update cell color
    cell.updateColour(lf);
    
    // Draw cell background
    g.setColour(cell.colour);
    g.fillEllipse(bounds.reduced(2).toFloat());
    
    // Draw cell border
    g.setColour(lf.getGridCellBorderColour());
    g.drawEllipse(bounds.reduced(2).toFloat(), 1.0f);
    
    // Draw staccato indicator
    if (cell.isStaccato)
    {
        g.setColour(lf.getGridCellStaccatoColour());
        auto dotBounds = bounds.reduced(bounds.getWidth() * 0.4f);
        g.fillEllipse(dotBounds.toFloat());
    }
}

void GridSequencerComponent::resized()
{
    repaint();
}

void GridSequencerComponent::mouseDown(const juce::MouseEvent& e)
{
    int row, col;
    if (getCellFromPoint(e.position.toInt(), row, col)) {
        isDragging = true;
        dragStartRow = row;
        dragStartCol = col;
        dragStartY = e.position.y;
        handleCellClick(row, col, e);
    }
}

void GridSequencerComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDragging) return;
    
    int row, col;
    if (getCellFromPoint(e.position.toInt(), row, col)) {
        // If we've moved to a new cell
        if (row != dragStartRow || col != dragStartCol) {
            handleCellClick(row, col, e);
            dragStartRow = row;
            dragStartCol = col;
        }
        
        // Update velocity based on vertical drag
        float dragDelta = (dragStartY - e.position.y) / getCellHeight();
        updateCellVelocity(row, col, dragDelta);
    }
}

void GridSequencerComponent::mouseUp(const juce::MouseEvent&)
{
    isDragging = false;
    dragStartRow = -1;
    dragStartCol = -1;
}

void GridSequencerComponent::handleCellClick(int row, int col, const juce::MouseEvent& e)
{
    GridCell& cell = grid[row][col];
    
    if (e.mods.isRightButtonDown()) {
        // Right click: cycle through accent levels
        updateCellAccent(row, col);
    } else if (e.mods.isAltDown()) {
        // Alt + click: toggle staccato
        if (cell.active) {
            cell.isStaccato = !cell.isStaccato;
            cell.updateColour();
        }
    } else {
        // Left click: toggle active state
        cell.active = !cell.active;
        if (cell.active) {
            cell.velocity = 100;
            cell.accent = 0;
        }
        cell.updateColour();
    }
    
    updatePattern();
    repaint();
}

void GridSequencerComponent::updateCellVelocity(int row, int col, float dragDelta)
{
    GridCell& cell = grid[row][col];
    if (!cell.active) return;
    
    // Update velocity based on drag amount
    cell.velocity = juce::jlimit(1, 127,
                               cell.velocity + static_cast<int>(dragDelta * 127));
    cell.updateColour();
    updatePattern();
    repaint();
}

void GridSequencerComponent::updateCellAccent(int row, int col)
{
    GridCell& cell = grid[row][col];
    if (!cell.active) return;
    
    // Cycle through accent levels (0 -> 1 -> 2 -> 0)
    cell.accent = (cell.accent + 1) % 3;
    cell.updateColour();
    updatePattern();
    repaint();
}

void GridSequencerComponent::timerCallback()
{
    if (processor.isPlaying()) {
        auto& pattern = processor.getCurrentPattern();
        int step = static_cast<int>(pattern.length * (processor.getCurrentPosition() / pattern.length)) % kCols;
        
        if (step != currentStep) {
            currentStep = step;
            repaint();
        }
    }
}

void GridSequencerComponent::updateFromPattern()
{
    const Pattern& pattern = processor.getCurrentPattern();
    
    // Clear grid
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell.active = false;
            cell.updateColour();
        }
    }
    
    // Update grid from pattern notes
    for (const auto& note : pattern.notes) {
        int col = static_cast<int>(note.startTime / pattern.gridSize);
        if (col >= 0 && col < kCols) {
            int row = note.pitch % kRows; // Simple mapping for now
            GridCell& cell = grid[row][col];
            cell.active = true;
            cell.velocity = note.velocity;
            cell.accent = note.accent;
            cell.isStaccato = note.isStaccato;
            cell.updateColour();
        }
    }
    
    repaint();
}

void GridSequencerComponent::updatePattern()
{
    Pattern newPattern;
    newPattern.length = kCols;
    newPattern.gridSize = 0.25; // 16th notes
    newPattern.tempo = processor.getCurrentPattern().tempo;
    
    // Convert grid to notes
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const GridCell& cell = grid[row][col];
            if (cell.active) {
                Note note;
                note.pitch = 60 + row; // Start from middle C
                note.startTime = col * newPattern.gridSize;
                note.duration = newPattern.gridSize;
                note.velocity = cell.velocity;
                note.accent = cell.accent;
                note.isStaccato = cell.isStaccato;
                note.isRest = false;
                newPattern.notes.push_back(note);
            }
        }
    }
    
    processor.setPattern(newPattern);
}

juce::Point<int> GridSequencerComponent::getCellPosition(int row, int col) const
{
    return { col * getCellWidth(), row * getCellHeight() };
}

juce::Rectangle<int> GridSequencerComponent::getCellBounds(int row, int col) const
{
    auto pos = getCellPosition(row, col);
    return { pos.x, pos.y, getCellWidth(), getCellHeight() };
}

bool GridSequencerComponent::getCellFromPoint(juce::Point<int> point, int& row, int& col) const
{
    col = point.x / getCellWidth();
    row = point.y / getCellHeight();
    
    return col >= 0 && col < kCols && row >= 0 && row < kRows;
}

void GridCell::updateColour(const GrooveSequencerLookAndFeel& lf)
{
    if (!active) {
        colour = lf.getBackgroundColour();
        baseColour = lf.getGridCellBorderColour().withAlpha(0.3f);
        ringColour = lf.getGridCellBorderColour().withAlpha(0.0f);
        return;
    }

    baseColour = accent ? lf.getBackgroundAccentColour() : lf.getGridCellBorderColour();
    baseColour = baseColour.withAlpha(velocity);
    
    ringColour = isStaccato ? lf.getGridCellStaccatoColour() : baseColour.withAlpha(0.0f);
    
    colour = baseColour;
}

void GridSequencerComponent::drawCell(juce::Graphics& g, int col, int row, const juce::Rectangle<int>& bounds)
{
    auto& cell = grid[row * kCols + col];
    
    // Draw cell background
    g.setColour(cell.baseColour);
    g.fillEllipse(bounds.toFloat());
    
    // Draw staccato ring if needed
    if (cell.isStaccato) {
        g.setColour(cell.ringColour);
        const float ringThickness = 2.0f;
        auto innerBounds = bounds.reduced(ringThickness).toFloat();
        g.drawEllipse(innerBounds, ringThickness);
    }
    
    // Draw accent indicator
    if (cell.accent) {
        g.setColour(cell.baseColour.brighter(0.2f));
        auto accentBounds = bounds.reduced(bounds.getWidth() * 0.3f);
        g.fillEllipse(accentBounds.toFloat());
    }
} 