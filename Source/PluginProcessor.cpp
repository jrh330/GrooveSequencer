#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs {
    const juce::String tempo{"tempo"};
    const juce::String gridSize{"gridSize"};
    const juce::String length{"length"};
}

GrooveSequencerAudioProcessor::GrooveSequencerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Parameters", Parameters::createParameterLayout()),
      currentPattern(static_cast<int>(Parameters::DEFAULT_LENGTH)),
      transformer(),
      loopMode(true),
      playing(false),
      currentPosition(0.0),
      sampleRate(44100.0),
      samplesPerBeat(0),
      currentStep(-1),
      currentGridSize(0.25),
      swingAmount(0.0),
      velocityScale(1.0),
      gateLength(0.5),
      patternModified(false),
      division(NoteDivision::Sixteenth),
      isRecording(false),
      transformationType(TransformationType::RandomInKey),
      rhythmPattern(RhythmPattern::Regular),
      articulationStyle(ArticulationStyle::Normal),
      midiBuffer(),
      floatBuffer(2, 512)  // Default buffer size
{
    // Set up file logger
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("GrooveSequencer")
                            .getChildFile("groove_sequencer.log");
    
    // Create directory if it doesn't exist
    if (!logFile.getParentDirectory().exists())
        logFile.getParentDirectory().createDirectory();
    
    // Create and set the logger (keeping last 10 logs, max 1MB each)
    fileLogger = std::make_unique<juce::FileLogger>(logFile, "GrooveSequencer Plugin Initialized");
    juce::Logger::setCurrentLogger(fileLogger.get());
    
    juce::Logger::writeToLog("GrooveSequencer plugin initialized");
    
    // Add parameter listeners
    state.addParameterListener(Parameters::TEMPO_ID, this);
    state.addParameterListener(Parameters::GRID_SIZE_ID, this);
    state.addParameterListener(Parameters::LENGTH_ID, this);
    state.addParameterListener(Parameters::SWING_ID, this);
    state.addParameterListener(Parameters::VELOCITY_ID, this);
    state.addParameterListener(Parameters::GATE_ID, this);

    // Initialize with a default empty pattern
    generateNewPattern();
    
    fileLogger->logMessage("Plugin initialized with default pattern length: " + juce::String(getLength()));
}

GrooveSequencerAudioProcessor::~GrooveSequencerAudioProcessor()
{
    juce::Logger::writeToLog("GrooveSequencer plugin shutting down");
    juce::Logger::setCurrentLogger(nullptr);
    
    state.removeParameterListener(Parameters::TEMPO_ID, this);
    state.removeParameterListener(Parameters::GRID_SIZE_ID, this);
    state.removeParameterListener(Parameters::LENGTH_ID, this);
    state.removeParameterListener(Parameters::SWING_ID, this);
    state.removeParameterListener(Parameters::VELOCITY_ID, this);
    state.removeParameterListener(Parameters::GATE_ID, this);
    
    if (playing)
        stopAllNotes();
}

void GrooveSequencerAudioProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    floatBuffer.setSize(2, samplesPerBlock);
    updatePlaybackPosition(0);
}

void GrooveSequencerAudioProcessor::releaseResources()
{
    stopAllNotes();
}

bool GrooveSequencerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    
    // Handle MIDI messages and start/stop notes
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            auto* voice = findFreeVoice();
            if (voice != nullptr) {
                voice->startNote(static_cast<float>(msg.getNoteNumber()), msg.getFloatVelocity());
                fileLogger->logMessage("Starting note: " + juce::String(msg.getNoteNumber()) + 
                                     " velocity: " + juce::String(msg.getFloatVelocity()));
            }
        }
        else if (msg.isNoteOff()) {
            for (auto& voice : voices) {
                if (voice.isActive() && voice.getCurrentNote() == msg.getNoteNumber()) {
                    voice.stopNote();
                    fileLogger->logMessage("Stopping note: " + juce::String(msg.getNoteNumber()));
                }
            }
        }
        else if (msg.isAllNotesOff()) {
            for (auto& voice : voices) {
                voice.stopNote();
            }
            fileLogger->logMessage("Stopping all notes");
        }
    }
    
    // Generate audio
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float currentSample = 0.0f;
        
        // Mix all active voices
        for (auto& voice : voices) {
            if (voice.isActive()) {
                currentSample += voice.getNextSample();
            }
        }
        
        // Apply simple limiter to prevent clipping
        currentSample = juce::jlimit(-0.8f, 0.8f, currentSample);
        
        leftChannel[sample] = currentSample;
        rightChannel[sample] = currentSample;
    }
    
    if (playing) {
        const auto numSamples = buffer.getNumSamples();
        updatePlaybackPosition(numSamples);
        
        if (isRecording) {
            // Handle incoming MIDI for recording
            for (const auto metadata : midiMessages) {
                handleMidiInput(metadata.getMessage());
            }
        }
    }
}

void GrooveSequencerAudioProcessor::updatePlaybackPosition(int numSamples)
{
    if (!playing) return;

    // Calculate timing values
    const double beatsPerMinute = getTempo();
    const double beatsPerSecond = beatsPerMinute / 60.0;
    const double samplesPerBeat = sampleRate / beatsPerSecond;
    
    // Calculate samples per step based on the division
    double divisionValue;
    switch (division) {
        case NoteDivision::Quarter: 
            divisionValue = 4.0; 
            break;
        case NoteDivision::Eighth: 
            divisionValue = 8.0; 
            break;
        case NoteDivision::Sixteenth: 
            divisionValue = 16.0; 
            break;
        default: 
            divisionValue = 16.0; 
            break;
    }
    
    const double samplesPerStep = samplesPerBeat / (divisionValue / 4.0); // Normalize to quarter notes
    
    // Add swing if enabled (only on even-numbered steps)
    const double swingOffset = (currentStep % 2 == 1) ? swingAmount * samplesPerStep * 0.5 : 0.0;
    
    // Update position
    currentPosition += numSamples;
    
    // Check if we need to advance to the next step
    if (currentPosition >= samplesPerStep + swingOffset)
    {
        currentPosition -= (samplesPerStep + swingOffset);
        const int previousStep = currentStep;
        currentStep++;
        
        // Handle loop point
        const int patternLength = static_cast<int>(currentPattern.getNotes().size());
        if (currentStep >= patternLength)
        {
            if (loopMode)
            {
                currentStep = 0;
                currentPosition = 0.0;
                fileLogger->logMessage("Pattern loop point reached, restarting from step 0");
            }
            else
            {
                stopPlayback();
                fileLogger->logMessage("End of pattern reached, stopping playback");
                return;
            }
        }
        
        fileLogger->logMessage("Step advanced: " + juce::String(previousStep) + " -> " + 
                             juce::String(currentStep) + 
                             " (position: " + juce::String(currentPosition, 2) + 
                             " samples, tempo: " + juce::String(beatsPerMinute, 1) + 
                             " BPM, swing: " + juce::String(swingAmount, 2) + ")");
        
        triggerNotesForCurrentStep();
    }
}

void GrooveSequencerAudioProcessor::triggerNotesForCurrentStep()
{
    const juce::ScopedLock sl(patternLock);
    const auto& notes = currentPattern.getNotes();
    
    if (currentStep < 0 || currentStep >= static_cast<int>(notes.size()))
    {
        fileLogger->logMessage("Invalid step index: " + juce::String(currentStep) + 
                             " (pattern size: " + juce::String(notes.size()) + ")");
        return;
    }

    const auto& note = notes[static_cast<size_t>(currentStep)];
    if (note.active)
    {
        // Stop any currently playing notes
        stopAllNotes();
        
        // Start the note with velocity scaling
        auto* voice = findFreeVoice();
        if (voice != nullptr)
        {
            float velocity = static_cast<float>((note.velocity / 127.0f) * velocityScale);
            voice->startNote(static_cast<float>(note.pitch), velocity);
            fileLogger->logMessage("Playing note: pitch=" + juce::String(note.pitch) + 
                               " velocity=" + juce::String(velocity) +
                               " accent=" + juce::String(note.accent) +
                               " staccato=" + juce::String(note.isStaccato ? 1 : 0) +
                               " at step " + juce::String(currentStep));
        }
        else
        {
            fileLogger->logMessage("No free voice available for step " + juce::String(currentStep));
        }
    }
    else
    {
        fileLogger->logMessage("Step " + juce::String(currentStep) + " is inactive");
    }
}

void GrooveSequencerAudioProcessor::sendNoteEvents()
{
    const juce::ScopedLock sl(patternLock);
    const auto& notes = currentPattern.getNotes();
    
    const int step = static_cast<int>(currentPosition);
    if (step != currentStep)
    {
        currentStep = step;
        stopAllNotes();
        
        if (step >= 0 && step < static_cast<int>(notes.size()))
        {
            const auto& note = notes[static_cast<size_t>(step)];
            if (note.active)  // Only send if note is active
            {
                const int velocity = static_cast<int>(note.velocity * velocityScale * 127.0f);
                const int channel = 1;
                const int noteNumber = note.pitch;
                
                // Apply accent if needed
                const float accentMultiplier = note.accent > 0 ? 1.2f : 1.0f;
                const int finalVelocity = static_cast<int>(juce::jlimit(1, 127, static_cast<int>(velocity * accentMultiplier)));
                
                midiBuffer.addEvent(juce::MidiMessage::noteOn(channel, noteNumber, static_cast<uint8>(finalVelocity)), 0);
                
                // Adjust gate length for staccato notes
                const float effectiveGateLength = note.isStaccato ? static_cast<float>(gateLength * 0.5) : static_cast<float>(gateLength);
                const int noteDuration = static_cast<int>(samplesPerBeat * effectiveGateLength);
                midiBuffer.addEvent(juce::MidiMessage::noteOff(channel, noteNumber), noteDuration);
                
                fileLogger->logMessage("Sent MIDI note: step=" + juce::String(step) + 
                                     " pitch=" + juce::String(noteNumber) + 
                                     " velocity=" + juce::String(finalVelocity) + 
                                     " accent=" + juce::String(note.accent) + 
                                     " staccato=" + juce::String(note.isStaccato ? 1 : 0) + 
                                     " duration=" + juce::String(noteDuration));
            }
        }
        else
        {
            fileLogger->logMessage("Invalid step index in sendNoteEvents: " + juce::String(step));
        }
    }
}

void GrooveSequencerAudioProcessor::stopAllNotes()
{
    for (auto& voice : voices) {
        if (voice.isActive()) {
            voice.stopNote();
        }
    }
}

void GrooveSequencerAudioProcessor::startPlayback()
{
    if (!playing) {
        playing = true;
        currentStep = -1;  // Will advance to 0 on first update
        currentPosition = 0.0;
        juce::Logger::writeToLog("Starting playback at tempo: " + juce::String(getTempo()));
    }
}

void GrooveSequencerAudioProcessor::stopPlayback()
{
    if (playing) {
        playing = false;
        currentStep = -1;
        currentPosition = 0.0;
        stopAllNotes();
        juce::Logger::writeToLog("Stopping playback");
    }
}

void GrooveSequencerAudioProcessor::setPattern(const Pattern& pattern)
{
    const juce::ScopedLock sl(patternLock);
    
    // Validate pattern
    if (pattern.getNotes().empty()) {
        fileLogger->logMessage("Warning: Attempting to set empty pattern");
        return;
    }
    
    currentPattern = pattern;
    patternModified = true;
    
    fileLogger->logMessage("Pattern set with " + juce::String(currentPattern.getNotes().size()) + " notes");
    
    // Log first few notes for debugging
    const auto& notes = currentPattern.getNotes();
    for (size_t i = 0; i < std::min(static_cast<size_t>(4), notes.size()); ++i) {
        const auto& note = notes[i];
        fileLogger->logMessage("Note " + juce::String(i) + ": pitch=" + juce::String(note.pitch) + 
                             " active=" + juce::String(note.active ? 1 : 0) + 
                             " velocity=" + juce::String(note.velocity));
    }
}

void GrooveSequencerAudioProcessor::setTransformationType(TransformationType type)
{
    const juce::ScopedLock sl(patternLock);
    auto transformed = transformer.transformPattern(currentPattern, type);
    currentPattern = transformed;
    patternModified = true;
}

void GrooveSequencerAudioProcessor::setRhythmPattern(RhythmPattern pattern)
{
    transformer.setRhythmPattern(pattern);
}

void GrooveSequencerAudioProcessor::setArticulationStyle(ArticulationStyle style)
{
    transformer.setArticulationStyle(style);
}

void GrooveSequencerAudioProcessor::generateNewPattern()
{
    const juce::ScopedLock sl(patternLock);
    auto pattern = transformer.generatePattern(transformationType, currentPattern.getNotes().size());
    currentPattern = pattern;
    patternModified = true;
}

void GrooveSequencerAudioProcessor::transformCurrentPattern()
{
    const juce::ScopedLock sl(patternLock);
    
    fileLogger->logMessage("Transforming pattern with type: " + getTransformationTypeString(transformationType));
    
    auto transformed = transformer.transformPattern(currentPattern, transformationType);
    currentPattern = transformed;
    patternModified = true;
    
    fileLogger->logMessage("Pattern transformed: " + juce::String(currentPattern.getNotes().size()) + " notes");
}

juce::String GrooveSequencerAudioProcessor::getTransformationTypeString(TransformationType type) const
{
    switch (type) {
        case TransformationType::StepUp: return "Step Up";
        case TransformationType::StepDown: return "Step Down";
        case TransformationType::UpTwoDownOne: return "Up 2 Down 1";
        case TransformationType::SkipOne: return "Skip One";
        case TransformationType::Arch: return "Arch";
        case TransformationType::Pendulum: return "Pendulum";
        case TransformationType::PowerChord: return "Power Chord";
        case TransformationType::RandomFree: return "Random Free";
        case TransformationType::RandomInKey: return "Random In Key";
        case TransformationType::RandomRhythmic: return "Random Rhythmic";
        case TransformationType::Invert: return "Invert";
        case TransformationType::Mirror: return "Mirror";
        case TransformationType::Retrograde: return "Retrograde";
        case TransformationType::Reverse: return "Reverse";
        case TransformationType::ShiftLeft: return "Shift Left";
        case TransformationType::ShiftRight: return "Shift Right";
        default: return "Unknown";
    }
}

void GrooveSequencerAudioProcessor::parameterChanged(const juce::String& parameterID, float)
{
    if (parameterID == Parameters::TEMPO_ID)
    {
        updatePlaybackPosition(0);
    }
    else if (parameterID == Parameters::GRID_SIZE_ID || parameterID == Parameters::LENGTH_ID)
    {
        generateNewPattern();
    }
    else if (parameterID == Parameters::SWING_ID || 
             parameterID == Parameters::VELOCITY_ID ||
             parameterID == Parameters::GATE_ID)
    {
        // These parameters affect playback in real-time, no need for additional handling
    }
}

void GrooveSequencerAudioProcessor::handleMidiInput(const juce::MidiMessage& message)
{
    if (!playing || currentStep < 0)  // Ignore messages if not playing or invalid step
        return;
        
    if (message.isNoteOn())
    {
        const int noteNumber = message.getNoteNumber();
        const float velocity = message.getFloatVelocity();
        
        const juce::ScopedLock sl(patternLock);
        auto& notes = currentPattern.getNotes();
        
        // Ensure valid step index
        if (currentStep >= static_cast<int>(notes.size()))
            return;
            
        // Create and initialize new note
        Note newNote;
        newNote.pitch = noteNumber;
        newNote.velocity = velocity;
        newNote.startTime = static_cast<float>(currentPosition);
        newNote.duration = 1.0;  // Default duration
        newNote.active = true;   // Ensure note is active
        newNote.accent = 0;      // No accent by default
        newNote.isStaccato = false;  // Not staccato by default
        
        notes[static_cast<size_t>(currentStep)] = newNote;
        patternModified = true;
        
        fileLogger->logMessage("MIDI input recorded: note=" + juce::String(noteNumber) + 
                             " velocity=" + juce::String(velocity) + 
                             " at step=" + juce::String(currentStep));
    }
}

void GrooveSequencerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto treeState = state.copyState();
    std::unique_ptr<juce::XmlElement> xml(treeState.createXml());
    copyXmlToBinary(*xml, destData);
}

void GrooveSequencerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        state.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* GrooveSequencerAudioProcessor::createEditor()
{
    return new GrooveSequencerAudioProcessorEditor(*this);
}

void GrooveSequencerAudioProcessor::setTempo(double newTempo)
{
    state.getParameter(Parameters::TEMPO_ID)->setValueNotifyingHost(newTempo / 300.0);
}

void GrooveSequencerAudioProcessor::setSwingAmount(double amount)
{
    swingAmount = juce::jlimit(0.0, 1.0, amount);
}

void GrooveSequencerAudioProcessor::setVelocityScale(double scale)
{
    velocityScale = juce::jlimit(0.0, 2.0, scale);
}

void GrooveSequencerAudioProcessor::setGateLength(double length)
{
    gateLength = juce::jlimit(0.0, 1.0, length);
}

void GrooveSequencerAudioProcessor::setLength(int newLength)
{
    state.getParameter(Parameters::LENGTH_ID)->setValueNotifyingHost(static_cast<float>(newLength));
    generateNewPattern();
}

void GrooveSequencerAudioProcessor::updateGridCell(int row, int col, bool active, float velocity, int accent, bool isStaccato)
{
    const juce::ScopedLock sl(patternLock);
    
    // Validate input parameters
    if (row < 0 || col < 0 || velocity < 0.0f || velocity > 1.0f || accent < 0)
    {
        fileLogger->logMessage("Invalid grid cell parameters: row=" + juce::String(row) + 
                             " col=" + juce::String(col) + 
                             " velocity=" + juce::String(velocity) + 
                             " accent=" + juce::String(accent));
        return;
    }
        
    // Calculate the note index based on row and column
    const int gridSize = static_cast<int>(state.getParameter(Parameters::GRID_SIZE_ID)->getValue());
    if (col >= gridSize)  // Invalid column
    {
        fileLogger->logMessage("Column " + juce::String(col) + " exceeds grid size " + juce::String(gridSize));
        return;
    }
        
    const int noteIndex = row * gridSize + col;
    auto& notes = currentPattern.getNotes();
    
    // Ensure the pattern has enough notes
    while (notes.size() <= static_cast<size_t>(noteIndex)) {
        Note newNote;
        newNote.pitch = 60 + row;  // Middle C + row offset
        newNote.startTime = static_cast<float>(notes.size() * currentGridSize);
        newNote.duration = static_cast<float>(currentGridSize);
        newNote.active = false;     // Initialize as inactive
        newNote.velocity = 0.8f;    // Default velocity
        newNote.accent = 0;         // No accent
        newNote.isStaccato = false; // Not staccato
        notes.push_back(newNote);
    }
    
    // Update the note properties
    Note& note = notes[static_cast<size_t>(noteIndex)];
    note.active = active;
    note.velocity = velocity;
    note.accent = accent;
    note.isStaccato = isStaccato;
    
    patternModified = true;
    
    fileLogger->logMessage("Updated grid cell: row=" + juce::String(row) + 
                         " col=" + juce::String(col) + 
                         " active=" + juce::String(active ? 1 : 0) + 
                         " velocity=" + juce::String(velocity) + 
                         " accent=" + juce::String(accent) + 
                         " staccato=" + juce::String(isStaccato ? 1 : 0));
}

double GrooveSequencerAudioProcessor::getTempo() const
{
    // Get the normalized value (0-1) and scale it to our tempo range (30-300)
    return state.getParameter(Parameters::TEMPO_ID)->getValue() * 270.0 + 30.0;
}

void GrooveSequencerAudioProcessor::timerCallback()
{
    // Update UI with current playback position
    if (playing)
    {
        const double beatsPerMinute = getTempo();
        const double beatsPerSecond = beatsPerMinute / 60.0;
        const double samplesPerBeat = sampleRate / beatsPerSecond;
        
        // Calculate progress through current step
        const double stepProgress = currentPosition / samplesPerBeat;
        
        fileLogger->logMessage("Playback progress: step=" + juce::String(currentStep) + 
                             " progress=" + juce::String(stepProgress, 3));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveSequencerAudioProcessor();
}