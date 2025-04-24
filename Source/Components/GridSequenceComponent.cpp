#include "GridSequenceComponent.h"

//==============================================================================
GridSequenceComponent::GridSequenceComponent()
{
    // Initialize step properties
    stepProperties.resize(32); // Maximum possible steps
    
    // Create viewport and container for steps
    addAndMakeVisible(viewport = std::make_unique<juce::Viewport>());
    viewport->setViewedComponent(stepsContainer = std::make_unique<juce::Component>(), false);
    stepsContainer->setSize(800, 200); // Initial size
    
    // Create controls
    addAndMakeVisible(numStepsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight));
    numStepsSlider->setRange(1, 32, 1);
    numStepsSlider->setValue(16);
    numStepsSlider->onValueChange = [this] { setNumSteps((int)numStepsSlider->getValue()); };
    
    addAndMakeVisible(snakeModeButton = std::make_unique<juce::ToggleButton>("Snake Mode"));
    snakeModeButton->onClick = [this] { setSnakeMode(snakeModeButton->getToggleState()); };
    
    addAndMakeVisible(gridDivisionCombo = std::make_unique<juce::ComboBox>("Grid Division"));
    gridDivisionCombo->addItem("1/4", 1);
    gridDivisionCombo->addItem("1/8", 2);
    gridDivisionCombo->addItem("1/16", 3);
    gridDivisionCombo->addItem("1/32", 4);
    gridDivisionCombo->setSelectedId(3); // Default to 1/16
    gridDivisionCombo->onChange = [this] {
        switch (gridDivisionCombo->getSelectedId()) {
            case 1: gridDivision = 1.0; break;    // Quarter note
            case 2: gridDivision = 0.5; break;    // Eighth note
            case 3: gridDivision = 0.25; break;   // Sixteenth note
            case 4: gridDivision = 0.125; break;  // Thirty-second note
        }
        updateStepComponents();
    };
    
    createStepComponents();
    startTimer(50); // 20fps update for playback position
}

GridSequenceComponent::~GridSequenceComponent()
{
    stopTimer();
}

void GridSequenceComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GridSequenceComponent::resized()
{
    auto area = getLocalBounds();
    
    // Layout controls at the top
    auto controlsArea = area.removeFromTop(30);
    numStepsSlider->setBounds(controlsArea.removeFromLeft(200));
    controlsArea.removeFromLeft(10); // spacing
    snakeModeButton->setBounds(controlsArea.removeFromLeft(100));
    controlsArea.removeFromLeft(10); // spacing
    gridDivisionCombo->setBounds(controlsArea.removeFromLeft(100));
    
    // Main grid area
    viewport->setBounds(area);
    updateStepLayout();
}

void GridSequenceComponent::timerCallback()
{
    // Update playback position visualization
    for (auto* step : stepComponents)
        step->setPlaying(stepComponents.indexOf(step) == currentPlayStep);
}

void GridSequenceComponent::setPattern(const Pattern& pattern)
{
    // Update number of steps
    setNumSteps(pattern.length);
    
    // Update step properties from pattern notes
    for (size_t i = 0; i < pattern.notes.size() && i < stepProperties.size(); ++i) {
        auto& props = stepProperties[i];
        const auto& note = pattern.notes[i];
        props.enabled = true;
        props.pitch = note.pitch;
        props.velocity = note.velocity;
        props.duration = note.duration;
    }
    
    updateStepComponents();
}

Pattern GridSequenceComponent::getPattern() const
{
    Pattern pattern;
    pattern.length = numSteps;
    pattern.gridSize = gridDivision;
    
    double currentTime = 0.0;
    for (int i = 0; i < numSteps; ++i) {
        if (stepProperties[i].enabled) {
            Note note;
            note.startTime = currentTime;
            note.pitch = stepProperties[i].pitch;
            note.velocity = stepProperties[i].velocity;
            note.duration = stepProperties[i].duration;
            pattern.notes.push_back(note);
        }
        currentTime += gridDivision;
    }
    
    return pattern;
}

void GridSequenceComponent::setNumSteps(int steps)
{
    numSteps = juce::jlimit(1, 32, steps);
    numStepsSlider->setValue(numSteps, juce::dontSendNotification);
    updateStepLayout();
    
    if (onPatternChanged)
        onPatternChanged(getPattern());
}

void GridSequenceComponent::setSnakeMode(bool shouldSnake)
{
    snakeMode = shouldSnake;
    updateStepLayout();
}

void GridSequenceComponent::setStepPitch(int stepIndex, int pitch)
{
    if (stepIndex >= 0 && stepIndex < stepProperties.size()) {
        stepProperties[stepIndex].pitch = pitch;
        if (stepComponents[stepIndex])
            stepComponents[stepIndex]->setPitch(pitch);
            
        if (onPatternChanged)
            onPatternChanged(getPattern());
    }
}

void GridSequenceComponent::setStepVelocity(int stepIndex, int velocity)
{
    if (stepIndex >= 0 && stepIndex < stepProperties.size()) {
        stepProperties[stepIndex].velocity = velocity;
        if (stepComponents[stepIndex])
            stepComponents[stepIndex]->setVelocity(velocity);
            
        if (onPatternChanged)
            onPatternChanged(getPattern());
    }
}

void GridSequenceComponent::setStepDuration(int stepIndex, double duration)
{
    if (stepIndex >= 0 && stepIndex < stepProperties.size()) {
        stepProperties[stepIndex].duration = duration;
        if (stepComponents[stepIndex])
            stepComponents[stepIndex]->setDuration(duration);
            
        if (onPatternChanged)
            onPatternChanged(getPattern());
    }
}

void GridSequenceComponent::setStepEnabled(int stepIndex, bool enabled)
{
    if (stepIndex >= 0 && stepIndex < stepProperties.size()) {
        stepProperties[stepIndex].enabled = enabled;
        if (stepComponents[stepIndex])
            stepComponents[stepIndex]->setEnabled(enabled);
            
        if (onPatternChanged)
            onPatternChanged(getPattern());
    }
}

void GridSequenceComponent::setPlaybackPosition(int step)
{
    currentPlayStep = step;
}

void GridSequenceComponent::clearPlaybackPosition()
{
    currentPlayStep = -1;
}

void GridSequenceComponent::createStepComponents()
{
    stepComponents.clear();
    
    for (int i = 0; i < 32; ++i) {
        auto step = new StepComponent();
        step->onClick = [this, i] { handleStepClick(i); };
        step->onPitchChange = [this, i] { handleStepPropertyChange(i); };
        step->onVelocityChange = [this, i] { handleStepPropertyChange(i); };
        step->onDurationChange = [this, i] { handleStepPropertyChange(i); };
        
        stepsContainer->addAndMakeVisible(step);
        stepComponents.add(step);
    }
    
    updateStepComponents();
}

void GridSequenceComponent::updateStepComponents()
{
    // Update properties
    for (int i = 0; i < stepComponents.size(); ++i) {
        auto* step = stepComponents[i];
        const auto& props = stepProperties[i];
        
        step->setEnabled(props.enabled);
        step->setPitch(props.pitch);
        step->setVelocity(props.velocity);
        step->setDuration(props.duration);
        step->setVisible(i < numSteps);
    }
    
    updateStepLayout();
}

void GridSequenceComponent::updateStepLayout()
{
    const int totalWidth = (stepSize + stepSpacing) * maxStepsPerRow - stepSpacing;
    const int rowHeight = stepSize + stepSpacing;
    const int numRows = getRowCount();
    
    stepsContainer->setSize(totalWidth, rowHeight * numRows);
    
    for (int i = 0; i < numSteps; ++i) {
        auto pos = getStepPosition(i);
        stepComponents[i]->setBounds(pos.x, pos.y, stepSize, stepSize);
    }
}

int GridSequenceComponent::getRowCount() const
{
    return (numSteps + maxStepsPerRow - 1) / maxStepsPerRow;
}

juce::Point<int> GridSequenceComponent::getStepPosition(int stepIndex) const
{
    if (!snakeMode || numSteps <= maxStepsPerRow) {
        // Simple grid layout
        const int row = stepIndex / maxStepsPerRow;
        const int col = stepIndex % maxStepsPerRow;
        return { col * (stepSize + stepSpacing), row * (stepSize + stepSpacing) };
    } else {
        // Snake pattern layout
        const int row = stepIndex / maxStepsPerRow;
        int col;
        if (row % 2 == 0) {
            // Left to right
            col = stepIndex % maxStepsPerRow;
        } else {
            // Right to left
            col = maxStepsPerRow - 1 - (stepIndex % maxStepsPerRow);
        }
        return { col * (stepSize + stepSpacing), row * (stepSize + stepSpacing) };
    }
}

void GridSequenceComponent::handleStepClick(int stepIndex)
{
    setStepEnabled(stepIndex, !stepProperties[stepIndex].enabled);
}

void GridSequenceComponent::handleStepPropertyChange(int stepIndex)
{
    auto* step = stepComponents[stepIndex];
    if (step) {
        stepProperties[stepIndex].pitch = step->getPitch();
        stepProperties[stepIndex].velocity = step->getVelocity();
        stepProperties[stepIndex].duration = step->getDuration();
        
        if (onPatternChanged)
            onPatternChanged(getPattern());
    }
}

juce::MidiBuffer GridSequenceComponent::createMIDIBufferFromPattern() const
{
    juce::MidiBuffer buffer;
    
    // Convert pattern to MIDI events
    double samplesPerBeat = 960.0; // Standard MIDI PPQ
    double currentTime = 0.0;
    
    for (int i = 0; i < numSteps; ++i) {
        if (stepProperties[i].enabled) {
            // Note On
            juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, // Channel 1
                                                               stepProperties[i].pitch,
                                                               (uint8)stepProperties[i].velocity);
            buffer.addEvent(noteOn, static_cast<int>(currentTime * samplesPerBeat));
            
            // Note Off
            juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1,
                                                                 stepProperties[i].pitch,
                                                                 (uint8)0);
            buffer.addEvent(noteOff, static_cast<int>((currentTime + stepProperties[i].duration) * samplesPerBeat));
        }
        currentTime += gridDivision;
    }
    
    return buffer;
}

void GridSequenceComponent::startDragging()
{
    // Create a thumbnail image of the sequence
    int width = 100;
    int height = 50;
    juce::Image dragImage(juce::Image::ARGB, width, height, true);
    juce::Graphics g(dragImage);
    
    // Draw a simplified version of the sequence
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::lightblue);
    
    float stepWidth = width / static_cast<float>(numSteps);
    for (int i = 0; i < numSteps; ++i) {
        if (stepProperties[i].enabled) {
            float x = i * stepWidth;
            float y = height * (1.0f - stepProperties[i].velocity / 127.0f);
            g.fillRect(x, y, stepWidth - 1, height - y);
        }
    }
    
    // Start the drag operation
    juce::var midiData;
    auto buffer = createMIDIBufferFromPattern();
    juce::MemoryBlock block;
    juce::MidiFile midiFile;
    juce::MidiMessageSequence sequence;
    
    // Convert buffer to sequence
    for (const auto metadata : buffer) {
        sequence.addEvent(metadata.getMessage());
    }
    
    midiFile.addTrack(sequence);
    midiFile.writeTo(block);
    midiData = block.getData();
    
    startDragging("MIDI Sequence", dragImage, this, midiData);
}

//==============================================================================
StepComponent::StepComponent()
{
    // Initialize sliders
    addAndMakeVisible(pitchSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow));
    pitchSlider->setRange(0, 127, 1);
    pitchSlider->setValue(60);
    pitchSlider->onValueChange = [this] { if (onPitchChange) onPitchChange(); };
    
    addAndMakeVisible(velocitySlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow));
    velocitySlider->setRange(0, 127, 1);
    velocitySlider->setValue(100);
    velocitySlider->onValueChange = [this] { if (onVelocityChange) onVelocityChange(); };
    
    addAndMakeVisible(durationSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight));
    durationSlider->setRange(0.0, 4.0, 0.25);
    durationSlider->setValue(0.25);
    durationSlider->onValueChange = [this] { if (onDurationChange) onDurationChange(); };
}

StepComponent::~StepComponent() {}

void StepComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cornerSize = 4.0f;
    
    // Background
    g.setColour(enabled ? juce::Colours::darkblue : juce::Colours::darkgrey);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Drag highlight
    if (isBeingDraggedOver) {
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillRoundedRectangle(bounds, cornerSize);
    }
    
    // Border
    g.setColour(isPlaying ? juce::Colours::yellow : juce::Colours::grey);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    
    // Velocity indicator
    float velocityHeight = bounds.getHeight() * (velocity / 127.0f);
    g.setColour(juce::Colours::lightblue.withAlpha(0.5f));
    g.fillRect(bounds.removeFromBottom(velocityHeight));
}

void StepComponent::resized()
{
    // These components are hidden until the property popup is shown
    auto bounds = getLocalBounds();
    pitchSlider->setBounds(bounds.removeFromLeft(60));
    velocitySlider->setBounds(bounds.removeFromLeft(60));
    durationSlider->setBounds(bounds);
}

void StepComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
        showPropertyPopup();
    else if (onClick)
        onClick();
}

void StepComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (e.mouseWasDraggedSinceMouseDown()) {
        if (auto* parent = dynamic_cast<GridSequenceComponent*>(getParentComponent())) {
            parent->startDragging();
        }
    } else {
        // Handle existing velocity adjustment
        if (enabled && !e.mods.isPopupMenu()) {
            float newVelocity = juce::jlimit(0, 127, 127 - (int)(e.y * 127.0f / getHeight()));
            setVelocity(newVelocity);
        }
    }
}

void StepComponent::setEnabled(bool shouldBeEnabled)
{
    enabled = shouldBeEnabled;
    repaint();
}

void StepComponent::setPitch(int newPitch)
{
    pitch = newPitch;
    pitchSlider->setValue(pitch, juce::dontSendNotification);
    repaint();
}

void StepComponent::setVelocity(int newVelocity)
{
    velocity = newVelocity;
    velocitySlider->setValue(velocity, juce::dontSendNotification);
    repaint();
}

void StepComponent::setDuration(double newDuration)
{
    duration = newDuration;
    durationSlider->setValue(duration, juce::dontSendNotification);
    repaint();
}

void StepComponent::setPlaying(bool isNowPlaying)
{
    if (isPlaying != isNowPlaying) {
        isPlaying = isNowPlaying;
        repaint();
    }
}

void StepComponent::showPropertyPopup()
{
    auto* dialog = new juce::DialogWindow("Step Properties",
                                        juce::Colours::darkgrey,
                                        true,
                                        false);
    
    auto content = std::make_unique<juce::Component>();
    content->setSize(300, 200);
    
    // Add controls to the dialog
    auto* pitch = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    pitch->setRange(0, 127, 1);
    pitch->setValue(this->pitch);
    pitch->setTextValueSuffix(" (MIDI Note)");
    pitch->onChange = [this, pitch] { setPitch((int)pitch->getValue()); };
    content->addAndMakeVisible(pitch);
    
    auto* velocity = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    velocity->setRange(0, 127, 1);
    velocity->setValue(this->velocity);
    velocity->setTextValueSuffix(" (Velocity)");
    velocity->onChange = [this, velocity] { setVelocity((int)velocity->getValue()); };
    content->addAndMakeVisible(velocity);
    
    auto* duration = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    duration->setRange(0.0, 4.0, 0.25);
    duration->setValue(this->duration);
    duration->setTextValueSuffix(" beats");
    duration->onChange = [this, duration] { setDuration(duration->getValue()); };
    content->addAndMakeVisible(duration);
    
    // Layout the controls
    pitch->setBounds(10, 10, 280, 50);
    velocity->setBounds(10, 70, 280, 50);
    duration->setBounds(10, 130, 280, 50);
    
    dialog->setContentOwned(content.release(), true);
    dialog->centreAroundComponent(this, dialog->getWidth(), dialog->getHeight());
    dialog->setVisible(true);
}

bool StepComponent::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    // Accept drops from other MIDI sources
    return dragSourceDetails.description.toString().startsWith("MIDI");
}

void StepComponent::itemDragEnter(const SourceDetails& /*dragSourceDetails*/)
{
    isBeingDraggedOver = true;
    repaint();
}

void StepComponent::itemDragExit(const SourceDetails& /*dragSourceDetails*/)
{
    isBeingDraggedOver = false;
    repaint();
}

void StepComponent::itemDropped(const SourceDetails& dragSourceDetails)
{
    isBeingDraggedOver = false;
    
    // Handle the dropped MIDI data
    if (auto* midiData = dragSourceDetails.description.getBinaryData()) {
        juce::MemoryInputStream inputStream(*midiData, false);
        juce::MidiFile midiFile;
        
        if (midiFile.readFrom(inputStream)) {
            // Process the first MIDI event to set our properties
            if (midiFile.getNumTracks() > 0) {
                auto* track = midiFile.getTrack(0);
                if (track->getNumEvents() > 0) {
                    auto event = track->getEventPointer(0)->message;
                    if (event.isNoteOn()) {
                        setPitch(event.getNoteNumber());
                        setVelocity(event.getVelocity());
                        setEnabled(true);
                    }
                }
            }
        }
    }
    
    repaint();
} 