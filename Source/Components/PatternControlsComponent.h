#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../PatternTransformer.h"

class PatternControlsComponent : public juce::Component
{
public:
    PatternControlsComponent();
    ~PatternControlsComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Pattern selection
    void setCurrentPattern(const Pattern& pattern);
    Pattern getCurrentPattern() const;
    
    // Callbacks
    std::function<void(const Pattern&)> onPatternSelected;
    std::function<void(TransformationType)> onTransformationSelected;
    std::function<void(RhythmPattern)> onRhythmPatternSelected;
    std::function<void(ArticulationStyle)> onArticulationStyleSelected;

private:
    // UI Components
    std::unique_ptr<juce::ComboBox> transformationSelector;
    std::unique_ptr<juce::ComboBox> rhythmPatternSelector;
    std::unique_ptr<juce::ComboBox> articulationStyleSelector;
    std::unique_ptr<juce::Slider> humanizeSlider;
    std::unique_ptr<juce::TextButton> applyButton;
    std::unique_ptr<juce::TextButton> clearButton;
    
    // Pattern data
    Pattern currentPattern;
    
    // Helper methods
    void initializeTransformationSelector();
    void initializeRhythmPatternSelector();
    void initializeArticulationStyleSelector();
    void initializeHumanizeControls();
    void initializeButtons();
    
    void handleTransformationChange();
    void handleRhythmPatternChange();
    void handleArticulationStyleChange();
    void handleHumanizeChange();
    void handleApplyButton();
    void handleClearButton();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternControlsComponent)
}; 