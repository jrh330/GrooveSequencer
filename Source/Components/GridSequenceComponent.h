#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

// Forward declarations
class StepComponent;

class GridSequenceComponent : public juce::Component,
                            public juce::Timer,
                            public juce::DragAndDropContainer
{
public:
    GridSequenceComponent();
    ~GridSequenceComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Pattern management
    void setPattern(const Pattern& pattern);
    Pattern getPattern() const;
    void setNumSteps(int steps);
    void setSnakeMode(bool shouldSnake);
    
    // Step properties
    void setStepPitch(int stepIndex, int pitch);
    void setStepVelocity(int stepIndex, int velocity);
    void setStepDuration(int stepIndex, double duration);
    void setStepEnabled(int stepIndex, bool enabled);
    
    // Playback
    void setPlaybackPosition(int step);
    void clearPlaybackPosition();
    
    // MIDI Export
    void exportToMIDI();
    void setMIDIChannel(int channel);
    juce::MidiBuffer createMIDIBuffer() const;
    
    // Layout
    void updateStepLayout();
    
    // Callbacks
    std::function<void(const Pattern&)> onPatternChanged;
    std::function<void(const juce::MidiBuffer&)> onMIDIExport;

private:
    struct StepProperties {
        bool enabled = false;
        int pitch = 60;
        int velocity = 100;
        double duration = 0.25;  // In beats
    };

    // UI Components
    juce::OwnedArray<StepComponent> stepComponents;
    std::unique_ptr<juce::Viewport> viewport;
    std::unique_ptr<juce::Component> stepsContainer;
    
    // Controls
    std::unique_ptr<juce::Slider> numStepsSlider;
    std::unique_ptr<juce::ToggleButton> snakeModeButton;
    std::unique_ptr<juce::ComboBox> gridDivisionCombo;
    std::unique_ptr<juce::ComboBox> midiChannelCombo;
    std::unique_ptr<juce::TextButton> exportButton;
    
    // Pattern data
    std::vector<StepProperties> stepProperties;
    int numSteps = 16;
    bool snakeMode = false;
    int currentPlayStep = -1;
    double gridDivision = 0.25; // Quarter note = 1.0
    int midiChannel = 1;
    
    // Layout constants
    static constexpr int maxStepsPerRow = 16;
    static constexpr int stepSize = 40;
    static constexpr int stepSpacing = 4;
    
    // Helper methods
    void createStepComponents();
    void updateStepComponents();
    void handleStepClick(int stepIndex);
    void handleStepPropertyChange(int stepIndex);
    int getRowCount() const;
    juce::Point<int> getStepPosition(int stepIndex) const;
    void initializeMIDIControls();
    
    // Drag and Drop helpers
    juce::MidiBuffer createMIDIBufferFromPattern() const;
    void startDragging();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSequenceComponent)
};

class StepComponent : public juce::Component,
                     public juce::DragAndDropTarget
{
public:
    StepComponent();
    ~StepComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    
    // Properties
    void setEnabled(bool shouldBeEnabled);
    void setPitch(int newPitch);
    void setVelocity(int newVelocity);
    void setDuration(double newDuration);
    void setPlaying(bool isPlaying);
    
    // Getters
    bool isEnabled() const { return enabled; }
    int getPitch() const { return pitch; }
    int getVelocity() const { return velocity; }
    double getDuration() const { return duration; }
    
    // Drag and Drop
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;
    
    // Callbacks
    std::function<void()> onClick;
    std::function<void()> onPitchChange;
    std::function<void()> onVelocityChange;
    std::function<void()> onDurationChange;
    std::function<void()> onDragStart;

private:
    bool enabled = false;
    int pitch = 60;
    int velocity = 100;
    double duration = 0.25;
    bool isPlaying = false;
    bool isBeingDraggedOver = false;
    
    // UI Components for property editing
    std::unique_ptr<juce::Slider> pitchSlider;
    std::unique_ptr<juce::Slider> velocitySlider;
    std::unique_ptr<juce::Slider> durationSlider;
    
    void showPropertyPopup();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepComponent)
}; 