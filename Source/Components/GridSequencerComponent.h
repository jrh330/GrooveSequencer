#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include <vector>

// Forward declarations
class GrooveSequencerAudioProcessor;
class GrooveSequencerLookAndFeel;

/**
 * @brief Represents a single cell in the grid sequencer
 */
struct GridCell {
    bool active = false;
    float velocity = 100.0f;
    int accent = 0;
    bool isStaccato = false;
    
    void updateState(bool newActive, float newVelocity, int newAccent, bool newStaccato);
    juce::Colour getBaseColour(const GrooveSequencerLookAndFeel& lf) const;
    juce::Colour getRingColour(const GrooveSequencerLookAndFeel& lf) const;
};

/**
 * @brief A component that displays and handles interaction with a grid-based sequencer
 */
class GridSequencerComponent : public juce::Component,
                             public juce::Timer
{
public:
    static constexpr int kRows = 1;  // Changed from 4 to 1
    static constexpr int kCols = 16;
    
    GridSequencerComponent(GrooveSequencerAudioProcessor& p);
    ~GridSequencerComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    
    void timerCallback() override;
    
    // Get cell dimensions
    float getCellWidth() const { return static_cast<float>(getWidth()) / kCols; }
    float getCellHeight() const { return static_cast<float>(getHeight()) / kRows; }
    
    // Grid control methods
    void updateGridSize(int newSize);
    void setBasePitch(int midiNote);
    
private:
    GrooveSequencerAudioProcessor& processor;
    std::vector<std::vector<GridCell>> grid;
    
    // Grid state
    int basePitch = 60;  // Middle C
    int gridSize = 16;   // Default grid size
    
    // Mouse interaction state
    struct DragState {
        bool isDragging = false;
        int startRow = 0;
        int startCol = 0;
        float startY = 0.0f;
    } dragState;
    
    int currentStep = 0;
    
    // Mouse interaction
    void handleCellInteraction(int row, int col, const juce::ModifierKeys& mods, float dragDelta = 0.0f);
    
    // Grid helpers
    juce::Rectangle<float> getCellBounds(int row, int col) const;
    bool getCellFromPoint(juce::Point<int> point, int& row, int& col) const;
    bool isPositionValid(int row, int col) const;
    
    // Drawing helpers
    void drawBackground(juce::Graphics& g) const;
    void drawGrid(juce::Graphics& g) const;
    void drawCell(juce::Graphics& g, int row, int col) const;
    void drawPlayhead(juce::Graphics& g) const;
    
    // Get the LookAndFeel as the correct type
    GrooveSequencerLookAndFeel& getLookAndFeelAs() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSequencerComponent)
}; 