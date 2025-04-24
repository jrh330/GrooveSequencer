#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

// Forward declarations
class GrooveSequencerAudioProcessor;
class GrooveSequencerLookAndFeel;

/**
 * @brief Represents a single cell in the grid sequencer
 */
struct GridCell {
    bool active{false};
    float velocity{1.0f};
    int accent{0};
    bool isStaccato{false};
    
    void updateState(bool newActive, float newVelocity, int newAccent, bool newStaccato);
    [[nodiscard]] juce::Colour getBaseColour(const GrooveSequencerLookAndFeel& lf) const;
    [[nodiscard]] juce::Colour getRingColour(const GrooveSequencerLookAndFeel& lf) const;
};

/**
 * @brief A component that displays and handles interaction with a grid-based sequencer
 */
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
    [[nodiscard]] float getCellWidth() const { return static_cast<float>(getWidth()) / static_cast<float>(kCols); }
    [[nodiscard]] float getCellHeight() const { return static_cast<float>(getHeight()) / static_cast<float>(kRows); }
    
private:
    GrooveSequencerAudioProcessor& processor;
    std::vector<std::vector<GridCell>> grid;
    
    // Mouse interaction state
    struct DragState {
        bool isDragging{false};
        int startRow{-1};
        int startCol{-1};
        float startY{0.0f};
    } dragState;
    
    int currentStep{0};
    
    // Mouse interaction
    void handleCellInteraction(int row, int col, const juce::ModifierKeys& mods, float dragDelta = 0.0f);
    
    // Grid helpers
    [[nodiscard]] juce::Rectangle<float> getCellBounds(int row, int col) const;
    [[nodiscard]] bool getCellFromPoint(juce::Point<int> point, int& row, int& col) const;
    [[nodiscard]] bool isPositionValid(int row, int col) const;
    
    // Drawing helpers
    void drawBackground(juce::Graphics& g) const;
    void drawGrid(juce::Graphics& g) const;
    void drawCell(juce::Graphics& g, int row, int col) const;
    void drawPlayhead(juce::Graphics& g) const;
    
    // Get the LookAndFeel as the correct type
    [[nodiscard]] GrooveSequencerLookAndFeel& getLookAndFeelAs() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSequencerComponent)
}; 