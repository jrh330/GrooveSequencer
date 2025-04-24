#include "PluginProcessor.h"

namespace IDs {
    const juce::String tempo{"tempo"};
    const juce::String gridSize{"gridSize"};
    const juce::String length{"length"};
}

GrooveSequencerAudioProcessor::GrooveSequencerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Parameters",
            {
                std::make_unique<juce::AudioParameterFloat>("tempo", "Tempo", 40.0f, 300.0f, 120.0f),
                std::make_unique<juce::AudioParameterInt>("gridSize", "Grid Size", 1, 32, 16),
                std::make_unique<juce::AudioParameterInt>("length", "Length", 1, 64, 16)
            })
{
    tempoParam = state.getRawParameterValue("tempo");
    gridSizeParam = state.getRawParameterValue("gridSize");
    lengthParam = state.getRawParameterValue("length");
    
    state.addParameterListener("tempo", this);
    state.addParameterListener("gridSize", this);
    state.addParameterListener("length", this);
}

GrooveSequencerAudioProcessor::~GrooveSequencerAudioProcessor()
{
    state.removeParameterListener("tempo", this);
    state.removeParameterListener("gridSize", this);
    state.removeParameterListener("length", this);
    
    if (playing)
        midiBuffer.clear();
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
    midiMessages.clear();
    
    if (!playing)
        return;
        
    const auto numSamples = buffer.getNumSamples();
    updatePlaybackPosition(numSamples);
    sendNoteEvents();
}

void GrooveSequencerAudioProcessor::updatePlaybackPosition(int numSamples)
{
    if (!playing)
        return;
        
    const double tempo = static_cast<double>(*tempoParam);
    samplesPerBeat = static_cast<int>(60.0 * sampleRate / tempo);
    
    currentPosition += static_cast<double>(numSamples) / static_cast<double>(samplesPerBeat);
    
    const int gridSize = static_cast<int>(*gridSizeParam);
    if (currentPosition >= gridSize)
    {
        if (loopMode)
            currentPosition = std::fmod(currentPosition, static_cast<double>(gridSize));
        else
            stopPlayback();
    }
}

void GrooveSequencerAudioProcessor::sendNoteEvents()
{
    const juce::ScopedLock sl(patternLock);
    
    const int step = static_cast<int>(currentPosition);
    if (step != currentStep)
    {
        currentStep = step;
        stopAllNotes();
        
        if (step < currentPattern.notes.size())
        {
            const auto& note = currentPattern.notes[step];
            if (note.isValid())
            {
                const int velocity = static_cast<int>(note.velocity * velocityScale * 127.0f);
                const int channel = 1;
                const int noteNumber = note.pitch;
                
                midiBuffer.addEvent(juce::MidiMessage::noteOn(channel, noteNumber, static_cast<uint8>(velocity)), 0);
                
                const int noteDuration = static_cast<int>(samplesPerBeat * gateLength);
                midiBuffer.addEvent(juce::MidiMessage::noteOff(channel, noteNumber), noteDuration);
            }
        }
    }
}

void GrooveSequencerAudioProcessor::stopAllNotes()
{
    for (int channel = 1; channel <= 16; ++channel)
        for (int note = 0; note < 128; ++note)
            midiBuffer.addEvent(juce::MidiMessage::noteOff(channel, note), 0);
}

void GrooveSequencerAudioProcessor::startPlayback()
{
    currentPosition = 0.0;
    currentStep = -1;
    playing = true;
}

void GrooveSequencerAudioProcessor::stopPlayback()
{
    playing = false;
    stopAllNotes();
}

void GrooveSequencerAudioProcessor::setPattern(const Pattern& pattern)
{
    const juce::ScopedLock sl(patternLock);
    currentPattern = pattern;
    patternModified = true;
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

void GrooveSequencerAudioProcessor::generateNewPattern(int length)
{
    const juce::ScopedLock sl(patternLock);
    auto pattern = transformer.generatePattern(TransformationType::RandomInKey, length);
    currentPattern = pattern;
    patternModified = true;
}

void GrooveSequencerAudioProcessor::transformCurrentPattern()
{
    const juce::ScopedLock sl(patternLock);
    auto transformed = transformer.transformPattern(currentPattern, TransformationType::RandomInKey);
    currentPattern = transformed;
    patternModified = true;
}

void GrooveSequencerAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "tempo")
        updatePlaybackPosition(0);
}

void GrooveSequencerAudioProcessor::handleIncomingMidiMessage(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        // Store note info for potential pattern input
        const int noteNumber = message.getNoteNumber();
        const float velocity = message.getFloatVelocity();
        
        // Example: Add note to pattern at current position
        Note newNote;
        newNote.pitch = noteNumber;
        newNote.velocity = velocity;
        newNote.startTime = currentPosition;
        newNote.duration = 1.0;  // Default duration
        
        const juce::ScopedLock sl(patternLock);
        if (currentStep < currentPattern.notes.size())
            currentPattern.notes[currentStep] = newNote;
        else
            currentPattern.notes.push_back(newNote);
            
        patternModified = true;
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
    return new juce::GenericAudioProcessorEditor(*this);
}

void GrooveSequencerAudioProcessor::setTempo(double newTempo)
{
    state.getParameter("tempo")->setValueNotifyingHost(newTempo / 300.0);
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

void GrooveSequencerAudioProcessor::updateGridCell(int row, int col, bool active, float velocity, int accent, bool isStaccato)
{
    const juce::ScopedLock sl(patternLock);
    
    // Calculate the note index based on row and column
    const int noteIndex = row * static_cast<int>(*gridSizeParam) + col;
    
    // Ensure the pattern has enough notes
    while (currentPattern.notes.size() <= static_cast<size_t>(noteIndex)) {
        Note newNote;
        newNote.pitch = 60 + row; // Middle C + row offset
        newNote.startTime = currentPattern.notes.size() * currentGridSize;
        newNote.duration = currentGridSize;
        currentPattern.notes.push_back(newNote);
    }
    
    // Update the note properties
    Note& note = currentPattern.notes[noteIndex];
    note.active = active;
    note.velocity = velocity;
    note.accent = accent;
    note.isStaccato = isStaccato;
    
    patternModified = true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveSequencerAudioProcessor();
}