#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Components/GridSequencerComponent.h"
#include "GrooveSequencerLookAndFeel.h"

//==============================================================================
GrooveSequencerAudioProcessorEditor::GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Initialize look and feel
    lookAndFeel = std::make_unique<GrooveSequencerLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    
    // Initialize grid sequencer
    gridSequencer = std::make_unique<GridSequencerComponent>(processor);
    addAndMakeVisible(gridSequencer.get());
    
    // Set up all UI components
    setupTransportControls();
    setupGridControls();
    setupArticulationControls();
    setupPatternControls();
    setupFileControls();
    
    // Create parameter attachments
    auto& state = processor.getState();
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, Parameters::TEMPO_ID, tempoSlider);
    swingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, Parameters::SWING_ID, swingSlider);
    velocityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, Parameters::VELOCITY_ID, velocityScaleSlider);
    gateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, Parameters::GATE_ID, gateLengthSlider);
    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, Parameters::LENGTH_ID, lengthSlider);
    
    // Set window size
    setSize(800, 600);
    
    // Start timer for UI updates
    startTimerHz(30);
}

GrooveSequencerAudioProcessorEditor::~GrooveSequencerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GrooveSequencerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GrooveSequencerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top section: Transport and tempo controls
    auto topSection = area.removeFromTop(80);
    auto transportSection = topSection.removeFromLeft(200);
    playStopButton.setBounds(transportSection.removeFromLeft(95));
    loopButton.setBounds(transportSection);
    
    auto tempoSection = topSection.removeFromLeft(200);
    tempoLabel.setBounds(tempoSection.removeFromTop(20));
    tempoSlider.setBounds(tempoSection);
    
    auto swingSection = topSection;
    swingLabel.setBounds(swingSection.removeFromTop(20));
    swingSlider.setBounds(swingSection);
    
    area.removeFromTop(10); // Spacing
    
    // Grid controls section
    auto gridControlsSection = area.removeFromTop(80);
    auto gridSizeSection = gridControlsSection.removeFromLeft(150);
    gridSizeLabel.setBounds(gridSizeSection.removeFromTop(20));
    gridSizeSelector.setBounds(gridSizeSection);
    
    auto divisionSection = gridControlsSection.removeFromLeft(150);
    divisionLabel.setBounds(divisionSection.removeFromTop(20));
    divisionSelector.setBounds(divisionSection);
    
    auto lengthSection = gridControlsSection;
    lengthLabel.setBounds(lengthSection.removeFromTop(20));
    lengthSlider.setBounds(lengthSection);
    
    area.removeFromTop(10); // Spacing
    
    // Main grid area
    auto gridArea = area.removeFromTop(300);
    gridSequencer->setBounds(gridArea);
    
    area.removeFromTop(10); // Spacing
    
    // Bottom controls
    auto bottomSection = area;
    
    // Articulation controls
    auto articulationSection = bottomSection.removeFromLeft(300);
    auto velocitySection = articulationSection.removeFromTop(40);
    velocityLabel.setBounds(velocitySection.removeFromLeft(60));
    velocityScaleSlider.setBounds(velocitySection);
    
    auto gateSection = articulationSection.removeFromTop(40);
    gateLengthLabel.setBounds(gateSection.removeFromLeft(60));
    gateLengthSlider.setBounds(gateSection);
    
    auto buttonSection = articulationSection.removeFromTop(30);
    staccatoButton.setBounds(buttonSection.removeFromLeft(100));
    accentButton.setBounds(buttonSection.removeFromLeft(100));
    
    bottomSection.removeFromLeft(10); // Spacing
    
    // Pattern controls
    auto patternSection = bottomSection.removeFromLeft(300);
    auto row = patternSection.removeFromTop(30);
    transformationLabel.setBounds(row.removeFromLeft(60));
    transformationTypeSelector.setBounds(row);
    
    row = patternSection.removeFromTop(30);
    rhythmLabel.setBounds(row.removeFromLeft(60));
    rhythmPatternSelector.setBounds(row);
    
    row = patternSection.removeFromTop(30);
    articulationLabel.setBounds(row.removeFromLeft(60));
    articulationStyleSelector.setBounds(row);
    
    row = patternSection.removeFromTop(30);
    generateButton.setBounds(row.removeFromLeft(145));
    transformButton.setBounds(row);
    
    bottomSection.removeFromLeft(10); // Spacing
    
    // File controls and MIDI monitor
    auto fileSection = bottomSection;
    auto fileButtons = fileSection.removeFromTop(30);
    saveButton.setBounds(fileButtons.removeFromLeft(145));
    loadButton.setBounds(fileButtons);
    
    fileSection.removeFromTop(10);
    midiInputLabel.setBounds(fileSection.removeFromTop(20));
    midiMonitor.setBounds(fileSection);
}

void GrooveSequencerAudioProcessorEditor::setupTransportControls()
{
    // Play/Stop button
    addAndMakeVisible(playStopButton);
    playStopButton.setButtonText("Play");
    playStopButton.onClick = [this]() {
        if (processor.isPlaying()) {
            processor.stopPlayback();
            playStopButton.setButtonText("Play");
        } else {
            processor.startPlayback();
            playStopButton.setButtonText("Stop");
        }
    };
    
    // Loop button
    addAndMakeVisible(loopButton);
    loopButton.setButtonText("Loop");
    loopButton.setToggleState(true, juce::dontSendNotification);
    loopButton.onClick = [this]() {
        processor.setLoopMode(loopButton.getToggleState());
    };
    
    // Tempo control
    addAndMakeVisible(tempoSlider);
    addAndMakeVisible(tempoLabel);
    tempoSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    tempoSlider.setRange(30.0, 300.0, 1.0);
    tempoLabel.setText("Tempo", juce::dontSendNotification);
    tempoLabel.setJustificationType(juce::Justification::centred);
    
    // Swing control
    addAndMakeVisible(swingSlider);
    addAndMakeVisible(swingLabel);
    swingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    swingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    swingSlider.setRange(0.0, 1.0, 0.01);
    swingLabel.setText("Swing", juce::dontSendNotification);
    swingLabel.setJustificationType(juce::Justification::centred);
}

void GrooveSequencerAudioProcessorEditor::setupGridControls()
{
    // Grid size selector
    addAndMakeVisible(gridSizeSelector);
    addAndMakeVisible(gridSizeLabel);
    gridSizeLabel.setText("Grid Size", juce::dontSendNotification);
    gridSizeLabel.setJustificationType(juce::Justification::centred);
    gridSizeSelector.addItemList({"4", "8", "16", "32"}, 1);
    gridSizeSelector.setSelectedId(3, juce::dontSendNotification); // Default to 16
    gridSizeSelector.onChange = [this]() { updateGridSize(); };
    
    // Division selector
    addAndMakeVisible(divisionSelector);
    addAndMakeVisible(divisionLabel);
    divisionLabel.setText("Division", juce::dontSendNotification);
    divisionLabel.setJustificationType(juce::Justification::centred);
    divisionSelector.addItemList({"1/4", "1/8", "1/16"}, 1);
    divisionSelector.setSelectedId(3, juce::dontSendNotification); // Default to 1/16
    divisionSelector.onChange = [this]() {
        const int selectedId = divisionSelector.getSelectedId();
        NoteDivision division = NoteDivision::Sixteenth;
        if (selectedId == 1) division = NoteDivision::Quarter;
        else if (selectedId == 2) division = NoteDivision::Eighth;
        processor.setNoteDivision(division);
    };
    
    // Length control
    addAndMakeVisible(lengthSlider);
    addAndMakeVisible(lengthLabel);
    lengthLabel.setText("Length", juce::dontSendNotification);
    lengthLabel.setJustificationType(juce::Justification::centred);
    lengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    lengthSlider.setRange(1, 64, 1);
}

void GrooveSequencerAudioProcessorEditor::setupArticulationControls()
{
    // Velocity control
    addAndMakeVisible(velocityScaleSlider);
    addAndMakeVisible(velocityLabel);
    velocityLabel.setText("Velocity", juce::dontSendNotification);
    velocityScaleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    velocityScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    velocityScaleSlider.setRange(0.0, 2.0, 0.01);
    
    // Gate length control
    addAndMakeVisible(gateLengthSlider);
    addAndMakeVisible(gateLengthLabel);
    gateLengthLabel.setText("Gate", juce::dontSendNotification);
    gateLengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gateLengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    gateLengthSlider.setRange(0.1, 1.0, 0.01);
    
    // Articulation buttons
    addAndMakeVisible(staccatoButton);
    staccatoButton.setButtonText("Staccato");
    staccatoButton.setToggleState(false, juce::dontSendNotification);
    
    addAndMakeVisible(accentButton);
    accentButton.setButtonText("Accent");
    accentButton.setToggleState(false, juce::dontSendNotification);
}

void GrooveSequencerAudioProcessorEditor::setupPatternControls()
{
    // Transformation controls
    addAndMakeVisible(transformationTypeSelector);
    addAndMakeVisible(transformationLabel);
    transformationLabel.setText("Transform", juce::dontSendNotification);
    transformationTypeSelector.addItemList({
        "Random In Key",
        "Invert",
        "Mirror",
        "Step Up",
        "Step Down"
    }, 1);
    
    // Rhythm pattern controls
    addAndMakeVisible(rhythmPatternSelector);
    addAndMakeVisible(rhythmLabel);
    rhythmLabel.setText("Rhythm", juce::dontSendNotification);
    rhythmPatternSelector.addItemList({
        "Four On Floor",
        "Basic Rock",
        "Offbeat",
        "Syncopated"
    }, 1);
    
    // Articulation style controls
    addAndMakeVisible(articulationStyleSelector);
    addAndMakeVisible(articulationLabel);
    articulationLabel.setText("Style", juce::dontSendNotification);
    articulationStyleSelector.addItemList({
        "Legato",
        "Staccato",
        "Accented",
        "Mixed"
    }, 1);
    
    // Pattern manipulation buttons
    addAndMakeVisible(generateButton);
    generateButton.setButtonText("Generate");
    generateButton.onClick = [this]() {
        processor.generateNewPattern();
    };
    
    addAndMakeVisible(transformButton);
    transformButton.setButtonText("Transform");
    transformButton.onClick = [this]() {
        processor.transformCurrentPattern();
    };
}

void GrooveSequencerAudioProcessorEditor::setupFileControls()
{
    // File operation buttons
    addAndMakeVisible(saveButton);
    saveButton.setButtonText("Save Pattern");
    saveButton.onClick = [this]() {
        auto fileChooser = std::make_unique<juce::FileChooser>(
            "Save Pattern",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.xml");

        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file != juce::File{})
                {
                    processor.savePattern(file);
                }
            });
    };
    
    addAndMakeVisible(loadButton);
    loadButton.setButtonText("Load Pattern");
    loadButton.onClick = [this]() {
        auto fileChooser = std::make_unique<juce::FileChooser>(
            "Load Pattern",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.xml");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file != juce::File{})
                {
                    processor.loadPattern(file);
                }
            });
    };
    
    // MIDI monitor
    addAndMakeVisible(midiInputLabel);
    midiInputLabel.setText("MIDI Input", juce::dontSendNotification);
    
    addAndMakeVisible(midiMonitor);
    midiMonitor.setMultiLine(true);
    midiMonitor.setReadOnly(true);
    midiMonitor.setCaretVisible(false);
}

void GrooveSequencerAudioProcessorEditor::timerCallback()
{
    // Update play/stop button state
    if (processor.isPlaying() != (playStopButton.getButtonText() == "Stop")) {
        playStopButton.setButtonText(processor.isPlaying() ? "Stop" : "Play");
    }
    
    // Update grid sequencer
    if (gridSequencer) {
        gridSequencer->repaint();
    }
}

void GrooveSequencerAudioProcessorEditor::updateGridSize()
{
    const int selectedId = gridSizeSelector.getSelectedId();
    int newSize = 16; // Default
    
    switch (selectedId) {
        case 1: newSize = 4; break;
        case 2: newSize = 8; break;
        case 3: newSize = 16; break;
        case 4: newSize = 32; break;
    }
    
    if (gridSequencer) {
        gridSequencer->updateGridSize(newSize);
    }
}

void GrooveSequencerAudioProcessorEditor::updateMidiMonitor(const juce::String& message)
{
    // Append message to MIDI monitor
    midiMonitor.moveCaretToEnd();
    midiMonitor.insertTextAtCaret(message + "\n");
    
    // Keep only the last 1000 characters
    if (midiMonitor.getText().length() > 1000) {
        midiMonitor.setText(midiMonitor.getText().substring(
            midiMonitor.getText().length() - 1000));
    }
    
    // Scroll to bottom
    midiMonitor.moveCaretToEnd();
}

void GrooveSequencerAudioProcessorEditor::handleComboBoxChange(juce::ComboBox* comboBox)
{
    if (comboBox == &divisionSelector)
    {
        const int selectedId = comboBox->getSelectedId();
        NoteDivision division = NoteDivision::Sixteenth;
        if (selectedId == 1) division = NoteDivision::Quarter;
        else if (selectedId == 2) division = NoteDivision::Eighth;
        processor.setNoteDivision(division);
    }
}

void GrooveSequencerAudioProcessorEditor::handleButtonClick(juce::Button* button)
{
    if (button == &generateButton)
    {
        processor.generateNewPattern();
    }
    else if (button == &saveButton)
    {
        auto fileChooser = std::make_unique<juce::FileChooser>(
            "Save Pattern",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.xml");

        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file != juce::File{})
                {
                    processor.savePattern(file);
                }
            });
    }
    else if (button == &loadButton)
    {
        auto fileChooser = std::make_unique<juce::FileChooser>(
            "Load Pattern",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.xml");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file != juce::File{})
                {
                    processor.loadPattern(file);
                }
            });
    }
}