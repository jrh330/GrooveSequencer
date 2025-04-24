#include <JuceHeader.h>
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
    , processor(p)
    , gridSequencer(p)
{
    // Apply custom look and feel
    lookAndFeel = std::make_unique<GrooveSequencerLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    
    // Set background color
    setOpaque(true);
    
    // Set up transport controls
    setupTransportControls();
    
    // Set up grid size controls
    setupGridSizeControls();
    
    // Set up grid controls
    setupGridControls();
    
    // Set up articulation controls
    setupArticulationControls();
    
    // Add child components
    addAndMakeVisible(gridSequencer);
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(tempoSlider);
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(syncButton);
    addAndMakeVisible(gridSizeSelector);
    addAndMakeVisible(gridSizeLabel);
    addAndMakeVisible(tripletButton);
    addAndMakeVisible(dottedButton);
    addAndMakeVisible(swingSlider);
    addAndMakeVisible(swingLabel);
    addAndMakeVisible(velocityScaleSlider);
    addAndMakeVisible(velocityScaleLabel);
    addAndMakeVisible(gateLengthSlider);
    addAndMakeVisible(gateLengthLabel);
    addAndMakeVisible(articulationLabel);
    addAndMakeVisible(articulationSelector);
    
    // Set initial size
    setSize(800, 600);
}

GrooveSequencerAudioProcessorEditor::~GrooveSequencerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GrooveSequencerAudioProcessorEditor::setupTransportControls()
{
    // Play button
    playButton.setButtonText("Play");
    playButton.onClick = [this]() { processor.startPlayback(); };
    
    // Stop button
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this]() { processor.stopPlayback(); };
    
    // Tempo slider
    tempoLabel.setText("Tempo", juce::dontSendNotification);
    tempoSlider.setRange(40.0, 240.0, 1.0);
    tempoSlider.setValue(processor.getCurrentPattern().tempo);
    tempoSlider.setTextValueSuffix(" BPM");
    tempoSlider.onValueChange = [this]() {
        processor.setTempo(tempoSlider.getValue());
    };
    
    // Sync button
    syncButton.setButtonText("Sync to DAW");
    syncButton.onClick = [this]() {
        bool shouldSync = syncButton.getToggleState();
        processor.setSyncToHost(shouldSync);
        tempoSlider.setEnabled(!shouldSync);
    };
}

void GrooveSequencerAudioProcessorEditor::setupGridSizeControls()
{
    // Grid size selector
    gridSizeLabel.setText("Grid", juce::dontSendNotification);
    
    gridSizeSelector.addItem("1/4", 1);
    gridSizeSelector.addItem("1/8", 2);
    gridSizeSelector.addItem("1/16", 3);
    gridSizeSelector.addItem("1/32", 4);
    gridSizeSelector.setSelectedId(3); // Default to 1/16
    
    gridSizeSelector.onChange = [this]() {
        updateGridSize();
    };
    
    // Triplet button
    tripletButton.setButtonText("Triplet");
    tripletButton.onClick = [this]() {
        updateGridSize();
    };
    
    // Dotted button
    dottedButton.setButtonText("Dotted");
    dottedButton.onClick = [this]() {
        if (dottedButton.getToggleState())
            tripletButton.setToggleState(false, juce::dontSendNotification);
        updateGridSize();
    };
    
    // Make triplet and dotted mutually exclusive
    tripletButton.onClick = [this]() {
        if (tripletButton.getToggleState())
            dottedButton.setToggleState(false, juce::dontSendNotification);
        updateGridSize();
    };
}

void GrooveSequencerAudioProcessorEditor::updateGridSize()
{
    double baseSize = 1.0;
    switch (gridSizeSelector.getSelectedId())
    {
        case 1: baseSize = 1.0; break;     // 1/4
        case 2: baseSize = 0.5; break;     // 1/8
        case 3: baseSize = 0.25; break;    // 1/16
        case 4: baseSize = 0.125; break;   // 1/32
    }
    
    if (tripletButton.getToggleState())
        baseSize *= 2.0/3.0;
    else if (dottedButton.getToggleState())
        baseSize *= 1.5;
    
    processor.setGridSize(baseSize);
}

void GrooveSequencerAudioProcessorEditor::setupGridControls()
{
    // Swing control
    swingLabel.setText("Swing", juce::dontSendNotification);
    swingSlider.setRange(0.0, 1.0, 0.01);
    swingSlider.setTextValueSuffix("%");
    swingSlider.onValueChange = [this]() {
        processor.setSwingAmount(swingSlider.getValue());
    };
    
    // Velocity scale control
    velocityScaleLabel.setText("Velocity", juce::dontSendNotification);
    velocityScaleSlider.setRange(0.0, 2.0, 0.01);
    velocityScaleSlider.setValue(1.0);
    velocityScaleSlider.onValueChange = [this]() {
        processor.setVelocityScale(velocityScaleSlider.getValue());
    };
    
    // Gate length control
    gateLengthLabel.setText("Gate", juce::dontSendNotification);
    gateLengthSlider.setRange(0.1, 1.0, 0.01);
    gateLengthSlider.setValue(0.8);
    gateLengthSlider.setTextValueSuffix("%");
    gateLengthSlider.onValueChange = [this]() {
        processor.setGateLength(gateLengthSlider.getValue());
    };
}

void GrooveSequencerAudioProcessorEditor::setupArticulationControls()
{
    articulationLabel.setText("Articulation", juce::dontSendNotification);
    
    articulationSelector.addItem("Legato", 1);
    articulationSelector.addItem("Staccato", 2);
    articulationSelector.addItem("Mixed", 3);
    articulationSelector.addItem("Pattern", 4);
    articulationSelector.setSelectedId(1); // Default to Legato
    
    articulationSelector.onChange = [this]() {
        auto style = static_cast<ArticulationStyle>(articulationSelector.getSelectedId() - 1);
        processor.setArticulationStyle(style);
    };
}

void GrooveSequencerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    const int margin = 8;
    const int topHeight = 100;
    const int rightWidth = 200;
    
    // Top section split into two rows
    auto topSection = area.removeFromTop(topHeight);
    
    // Transport section (top left)
    auto transportArea = topSection.removeFromLeft(topSection.getWidth() - rightWidth);
    auto transportRow1 = transportArea.removeFromTop(45);
    auto transportRow2 = transportArea.removeFromTop(45);
    
    // Transport Row 1: Play/Stop and Tempo
    playButton.setBounds(transportRow1.removeFromLeft(80).reduced(margin));
    stopButton.setBounds(transportRow1.removeFromLeft(80).reduced(margin));
    transportRow1.removeFromLeft(margin * 2); // Spacing
    
    auto tempoArea = transportRow1.removeFromLeft(250);
    tempoLabel.setBounds(tempoArea.removeFromLeft(50).reduced(margin));
    tempoSlider.setBounds(tempoArea.reduced(margin));
    syncButton.setBounds(transportRow1.removeFromLeft(100).reduced(margin));
    
    // Transport Row 2: Grid Size and Articulation Controls
    auto leftControls = transportRow2.removeFromLeft(transportRow2.getWidth() / 2);
    auto rightControls = transportRow2;
    
    // Grid Size Controls (left)
    gridSizeLabel.setBounds(leftControls.removeFromLeft(40).reduced(margin));
    gridSizeSelector.setBounds(leftControls.removeFromLeft(100).reduced(margin));
    leftControls.removeFromLeft(margin * 2); // Spacing
    
    auto modifierArea = leftControls.removeFromLeft(200);
    tripletButton.setBounds(modifierArea.removeFromLeft(95).reduced(margin));
    dottedButton.setBounds(modifierArea.reduced(margin));
    
    // Articulation Controls (right)
    articulationLabel.setBounds(rightControls.removeFromLeft(70).reduced(margin));
    articulationSelector.setBounds(rightControls.reduced(margin));
    
    // Right side controls
    auto rightSection = area.removeFromRight(rightWidth);
    
    // Draw modernist separator
    auto separatorArea = area.removeFromRight(margin * 2);
    
    // Control sections with modernist spacing
    auto controlHeight = 90;
    auto controlSpacing = margin * 2;
    
    // Swing controls
    auto swingArea = rightSection.removeFromTop(controlHeight);
    swingLabel.setBounds(swingArea.removeFromTop(25).reduced(margin));
    swingSlider.setBounds(swingArea.reduced(margin));
    rightSection.removeFromTop(controlSpacing);
    
    // Velocity controls
    auto velocityArea = rightSection.removeFromTop(controlHeight);
    velocityScaleLabel.setBounds(velocityArea.removeFromTop(25).reduced(margin));
    velocityScaleSlider.setBounds(velocityArea.reduced(margin));
    rightSection.removeFromTop(controlSpacing);
    
    // Gate controls
    auto gateArea = rightSection.removeFromTop(controlHeight);
    gateLengthLabel.setBounds(gateArea.removeFromTop(25).reduced(margin));
    gateLengthSlider.setBounds(gateArea.reduced(margin));
    
    // Main grid area with modernist margins
    gridSequencer.setBounds(area.reduced(margin * 2));
}

void GrooveSequencerAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto& lf = dynamic_cast<GrooveSequencerLookAndFeel&>(getLookAndFeel());
    g.fillAll(lf.offWhite);
    
    auto bounds = getLocalBounds().toFloat();
    
    // Draw modernist decorative elements
    // Top section separator
    g.setColour(lf.mint.withAlpha(0.2f));
    g.drawLine(0, 100, bounds.getWidth(), 100, 2.0f);
    
    // Right section separator
    float separatorX = bounds.getWidth() - 216;
    g.setColour(lf.mint.withAlpha(0.1f));
    g.drawLine(separatorX, 0, separatorX, bounds.getHeight(), 3.0f);
    
    // Draw geometric accents
    float circleSize = 40.0f;
    float margin = 8.0f;
    
    // Top left circle
    lf.drawModernistCircle(g, 
        juce::Rectangle<float>(margin, margin, circleSize, circleSize),
        lf.yellow.withAlpha(0.2f));
    
    // Top right circle
    lf.drawModernistCircle(g,
        juce::Rectangle<float>(bounds.getWidth() - circleSize - margin, margin,
                             circleSize, circleSize),
        lf.red.withAlpha(0.2f));
    
    // Bottom decorative lines
    g.setColour(lf.mint.withAlpha(0.1f));
    float lineY = bounds.getHeight() - margin * 3;
    g.drawLine(margin * 4, lineY, bounds.getWidth() - margin * 4, lineY, 2.0f);
} 