#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "PatternTransformer.h"

// New struct to hold sequence metadata
struct SequenceMetadata {
    juce::String name;
    juce::String trackName;
    int length;
    double tempo;
    juce::Time lastModified;
    std::vector<Note> notes;
};

// New component for sequence browser
class SequenceBrowserComponent : public juce::Component,
                               public juce::TableListBoxModel
{
public:
    SequenceBrowserComponent();
    ~SequenceBrowserComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    
    // Sequence management
    void addSequence(const SequenceMetadata& sequence);
    void refreshSequences();
    SequenceMetadata getSelectedSequence() const;
    
    // Callback when sequence is selected
    std::function<void(const SequenceMetadata&)> onSequenceSelected;
    
private:
    juce::TableListBox sequenceList;
    juce::TextButton importButton;
    juce::TextButton previewButton;
    juce::Label filterLabel;
    juce::TextEditor filterEdit;
    
    std::vector<SequenceMetadata> sequences;
    int selectedRow = -1;
    
    enum ColumnIds {
        Name = 1,
        Track,
        Type,
        Articulation,
        Modified
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequenceBrowserComponent)
};

class GridSequenceComponent : public juce::Component
{
public:
    GridSequenceComponent();
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void setPattern(const SequenceMetadata& pattern);
    SequenceMetadata getPattern() const;

private:
    SequenceMetadata currentPattern;
    int gridSize = 16;
    float cellWidth = 30;
    float cellHeight = 20;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridSequenceComponent)
};

class PatternPresetComponent : public juce::Component
{
public:
    PatternPresetComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    std::function<void(RhythmPattern)> onPatternSelected;

private:
    juce::ComboBox patternSelector;
    juce::Label patternLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternPresetComponent)
};

class TransformationControlsComponent : public juce::Component
{
public:
    TransformationControlsComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    std::function<void(TransformationType)> onTransformationSelected;
    std::function<void(ArticulationStyle)> onArticulationSelected;

private:
    juce::ComboBox transformationSelector;
    juce::ComboBox articulationSelector;
    juce::Label transformLabel;
    juce::Label articulationLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransformationControlsComponent)
};

class PlaybackControlsComponent : public juce::Component
{
public:
    PlaybackControlsComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;
    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void(double)> onTempoChanged;

private:
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Slider tempoSlider;
    juce::Label tempoLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackControlsComponent)
};

class GrooveSequencerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor&);
    ~GrooveSequencerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GrooveSequencerAudioProcessor& audioProcessor;
    PatternTransformer patternTransformer;
    
    // Main UI Components
    GridSequenceComponent sequenceGrid;
    PatternPresetComponent patternPresets;
    TransformationControlsComponent transformControls;
    PlaybackControlsComponent playbackControls;
    
    // New Sequence Browser Component
    SequenceBrowserComponent sequenceBrowser;
    juce::TabbedComponent mainTabs;
    
    // Additional UI elements
    juce::Slider humanizeSlider;
    juce::ComboBox gridSizeSelector;
    juce::TextButton savePatternButton;
    juce::TextButton loadPatternButton;
    
    // Methods for handling UI events
    void handlePatternSelected(RhythmPattern pattern);
    void handleTransformationSelected(TransformationType type);
    void handleArticulationSelected(ArticulationStyle style);
    void handlePlaybackControls();
    void handlePatternSaveLoad();
    void updatePatternDisplay();
    
    // New methods for sequence browser
    void handleSequenceSelected(const SequenceMetadata& sequence);
    void importSelectedSequence();
    void scanForSequences();
    void createSequenceVariant(const SequenceMetadata& source, 
                             ArticulationStyle newStyle,
                             RhythmPattern newPattern);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessorEditor)
}; 