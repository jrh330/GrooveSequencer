#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GridSequenceComponent::GridSequenceComponent()
{
    setSize(480, 320); // 16 steps * 30 pixels wide, 16 notes * 20 pixels high
}

void GridSequenceComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    
    // Draw grid lines
    g.setColour(juce::Colours::grey);
    for (int x = 0; x <= gridSize; ++x)
        g.drawVerticalLine(x * cellWidth, 0.0f, getHeight());
    
    for (int y = 0; y <= gridSize; ++y)
        g.drawHorizontalLine(y * cellHeight, 0.0f, getWidth());
    
    // Draw notes
    g.setColour(juce::Colours::orange);
    for (const auto& note : currentPattern.notes)
    {
        int x = (note.startTime / currentPattern.gridSize) * cellWidth;
        int y = getHeight() - (note.pitch - 36) * cellHeight; // Assuming MIDI note range 36-96
        g.fillRect(x, y, cellWidth * note.duration / currentPattern.gridSize, cellHeight);
    }
}

//==============================================================================
PatternPresetComponent::PatternPresetComponent()
{
    addAndMakeVisible(patternLabel);
    patternLabel.setText("Pattern Type:", juce::dontSendNotification);
    
    addAndMakeVisible(patternSelector);
    patternSelector.addItem("Regular", 1);
    patternSelector.addItem("Samba", 2);
    patternSelector.addItem("Bossa Nova", 3);
    patternSelector.addItem("Rumba", 4);
    patternSelector.addItem("Mambo", 5);
    patternSelector.addItem("Cha Cha", 6);
    patternSelector.addItem("Tango", 7);
    patternSelector.addItem("Salsa", 8);
    patternSelector.addItem("2-3 Clave", 9);
    patternSelector.addItem("3-2 Clave", 10);
    
    patternSelector.onChange = [this]()
    {
        if (onPatternSelected)
        {
            auto pattern = static_cast<RhythmPattern>(patternSelector.getSelectedId() - 1);
            onPatternSelected(pattern);
        }
    };
}

void PatternPresetComponent::resized()
{
    auto area = getLocalBounds();
    patternLabel.setBounds(area.removeFromLeft(100));
    patternSelector.setBounds(area.reduced(5));
}

//==============================================================================
TransformationControlsComponent::TransformationControlsComponent()
{
    addAndMakeVisible(transformLabel);
    transformLabel.setText("Transform:", juce::dontSendNotification);
    
    addAndMakeVisible(transformationSelector);
    transformationSelector.addItem("Step Up", 1);
    transformationSelector.addItem("Step Down", 2);
    transformationSelector.addItem("Up Two Down One", 3);
    transformationSelector.addItem("Skip One", 4);
    transformationSelector.addItem("Alternate Octave", 5);
    transformationSelector.addItem("Invert", 6);
    transformationSelector.addItem("Mirror", 7);
    transformationSelector.addItem("Retrograde", 8);
    
    addAndMakeVisible(articulationLabel);
    articulationLabel.setText("Articulation:", juce::dontSendNotification);
    
    addAndMakeVisible(articulationSelector);
    articulationSelector.addItem("Legato", 1);
    articulationSelector.addItem("Staccato", 2);
    articulationSelector.addItem("Mixed", 3);
    articulationSelector.addItem("Pattern", 4);
}

void TransformationControlsComponent::resized()
{
    auto area = getLocalBounds();
    auto row = area.removeFromTop(30);
    
    transformLabel.setBounds(row.removeFromLeft(100));
    transformationSelector.setBounds(row.reduced(5));
    
    row = area.removeFromTop(30);
    articulationLabel.setBounds(row.removeFromLeft(100));
    articulationSelector.setBounds(row.reduced(5));
}

//==============================================================================
PlaybackControlsComponent::PlaybackControlsComponent()
{
    addAndMakeVisible(playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this]() { if (onPlay) onPlay(); };
    
    addAndMakeVisible(stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this]() { if (onStop) onStop(); };
    
    addAndMakeVisible(tempoLabel);
    tempoLabel.setText("Tempo:", juce::dontSendNotification);
    
    addAndMakeVisible(tempoSlider);
    tempoSlider.setRange(40.0, 240.0, 1.0);
    tempoSlider.setValue(120.0);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    tempoSlider.onValueChange = [this]()
    {
        if (onTempoChanged)
            onTempoChanged(tempoSlider.getValue());
    };
}

void PlaybackControlsComponent::resized()
{
    auto area = getLocalBounds();
    auto row = area.removeFromTop(30);
    
    playButton.setBounds(row.removeFromLeft(60));
    row.removeFromLeft(5);
    stopButton.setBounds(row.removeFromLeft(60));
    row.removeFromLeft(20);
    tempoLabel.setBounds(row.removeFromLeft(50));
    tempoSlider.setBounds(row.reduced(5));
}

//==============================================================================
SequenceBrowserComponent::SequenceBrowserComponent()
{
    // Setup table
    addAndMakeVisible(sequenceList);
    sequenceList.setModel(this);
    sequenceList.setMultipleSelectionEnabled(false);
    
    // Add columns
    sequenceList.getHeader().addColumn("Name", Name, 150);
    sequenceList.getHeader().addColumn("Track", Track, 100);
    sequenceList.getHeader().addColumn("Type", Type, 100);
    sequenceList.getHeader().addColumn("Articulation", Articulation, 100);
    sequenceList.getHeader().addColumn("Modified", Modified, 100);
    
    // Setup filter
    addAndMakeVisible(filterLabel);
    filterLabel.setText("Filter:", juce::dontSendNotification);
    
    addAndMakeVisible(filterEdit);
    filterEdit.setTextToShowWhenEmpty("Search sequences...", juce::Colours::grey);
    filterEdit.onTextChange = [this]() { refreshSequences(); };
    
    // Setup buttons
    addAndMakeVisible(importButton);
    importButton.setButtonText("Import");
    importButton.onClick = [this]()
    {
        if (selectedRow >= 0 && selectedRow < sequences.size())
        {
            if (onSequenceSelected)
                onSequenceSelected(sequences[selectedRow]);
        }
    };
    
    addAndMakeVisible(previewButton);
    previewButton.setButtonText("Preview");
    previewButton.onClick = [this]()
    {
        // TODO: Implement preview functionality
    };
}

SequenceBrowserComponent::~SequenceBrowserComponent()
{
}

void SequenceBrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SequenceBrowserComponent::resized()
{
    auto area = getLocalBounds();
    
    // Top controls
    auto topControls = area.removeFromTop(30);
    filterLabel.setBounds(topControls.removeFromLeft(50));
    filterEdit.setBounds(topControls.removeFromLeft(200));
    importButton.setBounds(topControls.removeFromRight(80));
    previewButton.setBounds(topControls.removeFromRight(80));
    
    // Table
    sequenceList.setBounds(area);
}

int SequenceBrowserComponent::getNumRows()
{
    return sequences.size();
}

void SequenceBrowserComponent::paintRowBackground(juce::Graphics& g, int rowNumber,
                                                int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else if (rowNumber % 2)
        g.fillAll(juce::Colours::grey.withAlpha(0.1f));
}

void SequenceBrowserComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId,
                                       int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= sequences.size())
        return;
        
    g.setColour(rowIsSelected ? juce::Colours::darkblue : juce::Colours::black);
    
    const auto& sequence = sequences[rowNumber];
    String text;
    
    switch (columnId)
    {
        case Name:
            text = sequence.name;
            break;
        case Track:
            text = sequence.trackName;
            break;
        case Type:
            text = getRhythmPatternName(sequence.rhythmType);
            break;
        case Articulation:
            text = getArticulationStyleName(sequence.articulationType);
            break;
        case Modified:
            text = sequence.lastModified.toString(true, true, true, true);
            break;
    }
    
    g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft);
}

void SequenceBrowserComponent::addSequence(const SequenceInfo& sequence)
{
    sequences.push_back(sequence);
    sequenceList.updateContent();
}

void SequenceBrowserComponent::refreshSequences()
{
    String filter = filterEdit.getText().toLowerCase();
    
    // TODO: Implement filtering based on search text
    
    sequenceList.updateContent();
}

SequenceInfo SequenceBrowserComponent::getSelectedSequence() const
{
    if (selectedRow >= 0 && selectedRow < sequences.size())
        return sequences[selectedRow];
    
    return SequenceInfo();
}

//==============================================================================
GrooveSequencerAudioProcessorEditor::GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , mainTabs(juce::TabbedComponent::TabsAtTop)
{
    setSize(800, 600);
    
    // Setup tabs
    addAndMakeVisible(mainTabs);
    
    // Create main editor panel
    auto* editorPanel = new Component();
    editorPanel->addAndMakeVisible(sequenceGrid);
    editorPanel->addAndMakeVisible(patternPresets);
    editorPanel->addAndMakeVisible(transformControls);
    editorPanel->addAndMakeVisible(playbackControls);
    editorPanel->addAndMakeVisible(humanizeSlider);
    editorPanel->addAndMakeVisible(gridSizeSelector);
    editorPanel->addAndMakeVisible(savePatternButton);
    editorPanel->addAndMakeVisible(loadPatternButton);
    
    // Add tabs
    mainTabs.addTab("Editor", juce::Colours::grey, editorPanel, true);
    mainTabs.addTab("Sequence Browser", juce::Colours::grey, &sequenceBrowser, true);
    
    // Setup sequence browser callbacks
    sequenceBrowser.onSequenceSelected = [this](const SequenceInfo& sequence)
    {
        handleSequenceSelected(sequence);
    };
    
    // Setup callbacks
    patternPresets.onPatternSelected = [this](RhythmPattern pattern)
    {
        handlePatternSelected(pattern);
    };
    
    transformControls.onTransformationSelected = [this](TransformationType type)
    {
        handleTransformationSelected(type);
    };
    
    playbackControls.onPlay = [this]()
    {
        processorRef.startPlayback();
    };
    
    playbackControls.onStop = [this]()
    {
        processorRef.stopPlayback();
    };
}

GrooveSequencerAudioProcessorEditor::~GrooveSequencerAudioProcessorEditor()
{
}

void GrooveSequencerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GrooveSequencerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    mainTabs.setBounds(area);
    
    if (auto* editorPanel = dynamic_cast<Component*>(mainTabs.getTabContentComponent(0)))
    {
        // Layout for editor panel
        auto editorArea = editorPanel->getLocalBounds();
        
        // Top controls
        auto topControls = editorArea.removeFromTop(120);
        auto row = topControls.removeFromTop(40);
        patternPresets.setBounds(row);
        
        row = topControls.removeFromTop(40);
        transformControls.setBounds(row);
        
        row = topControls.removeFromTop(40);
        playbackControls.setBounds(row);
        
        // Main sequence grid
        sequenceGrid.setBounds(editorArea.removeFromTop(320));
        
        // Bottom controls
        auto bottomControls = editorArea.removeFromBottom(40);
        humanizeSlider.setBounds(bottomControls.removeFromLeft(200));
        gridSizeSelector.setBounds(bottomControls.removeFromLeft(100));
        savePatternButton.setBounds(bottomControls.removeFromLeft(100));
        loadPatternButton.setBounds(bottomControls.removeFromLeft(100));
    }
}

void GrooveSequencerAudioProcessorEditor::handlePatternSelected(RhythmPattern pattern)
{
    processorRef.setCurrentPattern(pattern);
    updatePatternDisplay();
}

void GrooveSequencerAudioProcessorEditor::handleTransformationSelected(TransformationType type)
{
    processorRef.setTransformation(type);
    updatePatternDisplay();
}

void GrooveSequencerAudioProcessorEditor::updatePatternDisplay()
{
    auto pattern = processorRef.getCurrentPattern();
    sequenceGrid.setPattern(pattern);
    repaint();
}

void GrooveSequencerAudioProcessorEditor::handleSequenceSelected(const SequenceInfo& sequence)
{
    // Import the selected sequence
    sequenceGrid.setPattern(sequence.pattern);
    
    // Update controls to match the sequence
    patternPresets.setSelectedPattern(sequence.rhythmType);
    transformControls.setSelectedArticulation(sequence.articulationType);
    
    // Switch to editor tab
    mainTabs.setCurrentTabIndex(0);
    
    updatePatternDisplay();
}

void GrooveSequencerAudioProcessorEditor::scanForSequences()
{
    // TODO: Scan the DAW for existing sequences in other tracks
    // This will require integration with the host DAW's API
}

void GrooveSequencerAudioProcessorEditor::createSequenceVariant(
    const SequenceInfo& source,
    ArticulationStyle newStyle,
    RhythmPattern newPattern)
{
    // Create a new sequence based on the source
    SequenceInfo variant = source;
    variant.name = source.name + " (Modified)";
    variant.articulationType = newStyle;
    variant.rhythmType = newPattern;
    variant.lastModified = Time::getCurrentTime();
    
    // Apply the new style and pattern
    // TODO: Implement pattern and articulation transformation
    
    // Add to browser
    sequenceBrowser.addSequence(variant);
} 