#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../GrooveSequencerLookAndFeel.h"

class GridCell {
public:
    bool active = false;
    int velocity = 100;
    int accent = 0;
    bool isStaccato = false;
    
    void updateColour(const GrooveSequencerLookAndFeel& lf) {
        if (active) {
            // Base color from accent level
            switch (accent) {
                case 0: baseColour = lf.mint; break;
                case 1: baseColour = lf.yellow; break;
                case 2: baseColour = lf.red; break;
            }
            
            // Modify alpha based on velocity
            float alpha = 0.3f + (velocity / 127.0f) * 0.7f;
            baseColour = baseColour.withAlpha(alpha);
            
            // Staccato cells get a different style
            if (isStaccato) {
                ringColour = baseColour;
                baseColour = baseColour.withAlpha(0.2f);
            } else {
                ringColour = baseColour.brighter(0.2f);
            }
        } else {
            baseColour = lf.black.withAlpha(0.1f);
            ringColour = lf.black.withAlpha(0.2f);
        }
    }
    
    juce::Colour baseColour;
    juce::Colour ringColour;
};

class GridSequencerComponent : public juce::Component,
                             public juce::Timer
{
public:
    GridSequencerComponent(GrooveSequencerAudioProcessor& p);
    ~GridSequencerComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    // Grid dimensions
    static constexpr int kRows = 4;
    static constexpr int kCols = 16;
    
    // Timer callback for playhead
    void timerCallback() override;
    
    // Update grid from pattern
    void updateFromPattern();
    
    // Get cell dimensions
    int getCellWidth() const { return getWidth() / kCols; }
    int getCellHeight() const { return getHeight() / kRows; }
    
private:
    GrooveSequencerAudioProcessor& processor;
    std::array<std::array<GridCell, kCols>, kRows> grid;
    
    // Mouse interaction
    void handleCellClick(int row, int col, const juce::MouseEvent& e);
    void updateCellVelocity(int row, int col, float dragDelta);
    void updateCellAccent(int row, int col);
    
    // Grid helpers
    juce::Point<int> getCellPosition(int row, int col) const;
    juce::Rectangle<int> getCellBounds(int row, int col) const;
    bool getCellFromPoint(juce::Point<int> point, int& row, int& col) const;
    
    // Drawing helpers
    void drawGrid(juce::Graphics& g);
    void drawCell(juce::Graphics& g, int row, int col);
    void drawPlayhead(juce::Graphics& g);
    void drawModernistBackground(juce::Graphics& g);
    void drawGridLines(juce::Graphics& g);
    
    // Current state
    int currentStep = 0;
    bool isDragging = false;
    int dragStartRow = -1;
    int dragStartCol = -1;
    float dragStartY = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSequencerComponent)
}; 