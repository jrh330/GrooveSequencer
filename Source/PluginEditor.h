#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/GridSequencerComponent.h"
#include "GrooveSequencerLookAndFeel.h"

class GrooveSequencerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor& p);
    ~GrooveSequencerAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    GrooveSequencerAudioProcessor& processor;
    std::unique_ptr<GrooveSequencerLookAndFeel> lookAndFeel;
    
    // Grid component
    GridSequencerComponent gridSequencer;
    
    // Transport controls
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Label tempoLabel;
    juce::Slider tempoSlider;
    juce::ToggleButton syncButton;
    
    // Grid size controls
    juce::Label gridSizeLabel;
    juce::ComboBox gridSizeSelector;
    juce::ToggleButton tripletButton;
    juce::ToggleButton dottedButton;
    
    // Articulation controls
    juce::Label articulationLabel{"", "Articulation"};
    juce::ComboBox articulationSelector;
    
    // Grid controls
    juce::Label swingLabel;
    juce::Slider swingSlider;
    juce::Label velocityScaleLabel;
    juce::Slider velocityScaleSlider;
    juce::Label gateLengthLabel;
    juce::Slider gateLengthSlider;
    
    void setupTransportControls();
    void setupGridSizeControls();
    void setupGridControls();
    void setupArticulationControls();
    void updateGridSize();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessorEditor)
}; 