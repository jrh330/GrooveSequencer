#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Forward declarations
class GrooveSequencerAudioProcessor;
class GrooveSequencerLookAndFeel;

struct GridCell {
    bool active{false};
    float velocity{1.0f};
    bool accent{false};
    bool isStaccato{false};
    juce::Colour colour;
    juce::Colour baseColour;
    juce::Colour ringColour;
    
    void updateColour(const GrooveSequencerLookAndFeel& lf);
};

class GridSequencerComponent : public juce::Component,
                             public juce::Timer
{
public:
    explicit GridSequencerComponent(GrooveSequencerAudioProcessor& p);
    ~GridSequencerComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void timerCallback() override;
    
    // Grid dimensions
    static constexpr int kRows = 4;
    static constexpr int kCols = 16;
    
    // Get cell dimensions
    int getCellWidth() const { return getWidth() / kCols; }
    int getCellHeight() const { return getHeight() / kRows; }
    
private:
    GrooveSequencerAudioProcessor& processor;
    std::vector<std::vector<GridCell>> grid;
    
    // Mouse interaction
    void handleCellClick(int row, int col, const juce::ModifierKeys& mods);
    void updateCellVelocity(int row, int col, float dragDelta);
    void updateCellAccent(int row, int col);
    
    // Grid helpers
    juce::Point<int> getCellPosition(int row, int col) const;
    juce::Rectangle<int> getCellBounds(int row, int col) const;
    bool getCellFromPoint(juce::Point<int> point, int& row, int& col) const;
    
    // Drawing helpers
    void drawGrid(juce::Graphics& g);
    void drawCell(juce::Graphics& g, int row, int col, const GridCell& cell);
    void drawPlayhead(juce::Graphics& g);
    void drawModernistBackground(juce::Graphics& g);
    void drawGridLines(juce::Graphics& g);
    
    // Current state
    int currentStep = 0;
    bool isDragging{false};
    int lastRow{-1};
    int lastCol{-1};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSequencerComponent)
}; 