#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

// Forward declarations to break circular dependencies
class GrooveSequencerAudioProcessor;
class GridSequencerComponent;
class GrooveSequencerLookAndFeel;

class GrooveSequencerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor&);
    ~GrooveSequencerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GrooveSequencerAudioProcessor& audioProcessor;
    std::unique_ptr<GrooveSequencerLookAndFeel> lookAndFeel;
    std::unique_ptr<GridSequencerComponent> gridSequencer;
    
    // Transport controls
    juce::TextButton playButton{"Play"};
    juce::TextButton stopButton{"Stop"};
    juce::Slider tempoSlider;
    juce::Label tempoLabel;
    
    // Grid controls
    juce::Slider swingSlider;
    juce::Label swingLabel;
    juce::Slider velocityScaleSlider;
    juce::Label velocityScaleLabel;
    juce::Slider gateLengthSlider;
    juce::Label gateLengthLabel;
    
    // Grid size controls
    juce::Label gridSizeLabel{"", "Grid Size"};
    juce::ComboBox gridSizeSelector;
    juce::ToggleButton tripletButton{"Triplet"};
    juce::ToggleButton dottedButton{"Dotted"};
    
    // Articulation controls
    juce::Label articulationLabel{"", "Articulation"};
    juce::ComboBox articulationSelector;
    
    // Transport controls
    juce::ToggleButton syncButton{"Sync to Host"};
    
    void setupTransportControls();
    void setupGridSizeControls();
    void setupGridControls();
    void setupArticulationControls();
    void updateGridSize();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessorEditor)
}; 