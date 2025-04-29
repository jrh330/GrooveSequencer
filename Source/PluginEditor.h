#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/GridSequencerComponent.h"
#include "Components/TransportComponent.h"
#include "Components/PatternControlsComponent.h"
#include "Components/PatternBrowserComponent.h"
#include "GrooveSequencerLookAndFeel.h"

class GrooveSequencerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::Timer
{
public:
    explicit GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor&);
    ~GrooveSequencerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    GrooveSequencerAudioProcessor& processor;
    std::unique_ptr<GrooveSequencerLookAndFeel> lookAndFeel;
    
    // Main components
    std::unique_ptr<GridSequencerComponent> gridSequencer;
    
    // Transport controls
    juce::TextButton playStopButton;
    juce::TextButton loopButton;
    juce::Label tempoLabel;
    juce::Slider tempoSlider;
    juce::Label swingLabel;
    juce::Slider swingSlider;
    
    // Grid controls
    juce::Label gridSizeLabel;
    juce::ComboBox gridSizeSelector;
    juce::Label divisionLabel;
    juce::ComboBox divisionSelector;
    juce::Label lengthLabel;
    juce::Slider lengthSlider;
    
    // Articulation controls
    juce::Label velocityLabel;
    juce::Slider velocityScaleSlider;
    juce::Label gateLengthLabel;
    juce::Slider gateLengthSlider;
    juce::ToggleButton staccatoButton;
    juce::ToggleButton accentButton;
    
    // Pattern controls
    juce::Label transformationLabel;
    juce::ComboBox transformationTypeSelector;
    juce::Label rhythmLabel;
    juce::ComboBox rhythmPatternSelector;
    juce::Label articulationLabel;
    juce::ComboBox articulationStyleSelector;
    juce::TextButton generateButton;
    juce::TextButton transformButton;
    
    // File controls
    juce::TextButton saveButton;
    juce::TextButton loadButton;
    juce::Label midiInputLabel;
    juce::TextEditor midiMonitor;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> swingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lengthAttachment;
    
    // Setup methods
    void setupTransportControls();
    void setupGridControls();
    void setupArticulationControls();
    void setupPatternControls();
    void setupFileControls();
    
    // Update methods
    void updateGridSize();
    void updatePlayState();
    void updateMidiMonitor(const juce::String& message);
    
    // Event handlers
    void handleComboBoxChange(juce::ComboBox* comboBox);
    void handleButtonClick(juce::Button* button);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessorEditor)
}; 