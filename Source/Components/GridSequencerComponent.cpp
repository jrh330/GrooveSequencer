#include "GridSequencerComponent.h"

GridSequencerComponent::GridSequencerComponent(GrooveSequencerAudioProcessor& p)
    : processor(p)
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
    
    // Fill background
    g.fillAll(lf.offWhite);
    
    // Draw modernist background elements
    drawModernistBackground(g);
    
    // Draw grid
    drawGrid(g);
    
    // Draw grid lines
    drawGridLines(g);
    
    // Draw playhead if playing
    if (processor.isPlaying())
        drawPlayhead(g);
}

void GridSequencerComponent::drawModernistBackground(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    auto bounds = getLocalBounds().toFloat();
    
    // Draw geometric background elements
    g.setColour(lf.mint.withAlpha(0.05f));
    
    // Left side vertical line
    float lineX = getCellWidth() * 4;
    g.drawLine(lineX, 0, lineX, bounds.getHeight(), 2.0f);
    
    // Right side vertical line
    lineX = getCellWidth() * 12;
    g.drawLine(lineX, 0, lineX, bounds.getHeight(), 2.0f);
    
    // Horizontal accent lines
    for (int row = 1; row < kRows; row += 2) {
        float y = row * getCellHeight();
        g.drawLine(0, y, bounds.getWidth(), y, 1.0f);
    }
}

void GridSequencerComponent::drawGrid(juce::Graphics& g)
{
    // Draw cells
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            drawCell(g, row, col);
        }
    }
}

void GridSequencerComponent::drawGridLines(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    const int cellW = getCellWidth();
    const int cellH = getCellHeight();
    
    // Draw vertical lines
    for (int col = 0; col <= kCols; ++col) {
        float x = col * cellW;
        float thickness = (col % 4 == 0) ? 2.0f : 1.0f;
        g.setColour(lf.black.withAlpha(col % 4 == 0 ? 0.3f : 0.1f));
        g.drawLine(x, 0.0f, x, (float)getHeight(), thickness);
    }
    
    // Draw horizontal lines
    for (int row = 0; row <= kRows; ++row) {
        float y = row * cellH;
        g.setColour(lf.black.withAlpha(0.1f));
        g.drawLine(0.0f, y, (float)getWidth(), y, 1.0f);
    }
}

void GridSequencerComponent::drawCell(juce::Graphics& g, int row, int col)
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

void GridSequencerComponent::drawPlayhead(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    const int cellW = getCellWidth();
    const float x = currentStep * cellW;
    
    // Draw playhead line
    g.setColour(lf.yellow.withAlpha(0.5f));
    g.drawLine(x, 0.0f, x, (float)getHeight(), 2.0f);
    
    // Draw playhead markers
    float markerSize = 6.0f;
    g.setColour(lf.yellow);
    g.fillEllipse(x - markerSize * 0.5f, -markerSize * 0.5f, markerSize, markerSize);
    g.fillEllipse(x - markerSize * 0.5f, getHeight() - markerSize * 0.5f, markerSize, markerSize);
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