#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdint>

GrooveSequencerAudioProcessor::GrooveSequencerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , currentStep(0)
    , playing(false)
    , sampleRate(44100.0)
    , tempo(120.0)
    , loopStartStep(0)
    , loopEndStep(15)
    , swingAmount(0.0)
    , velocityScale(1.0)
    , gateLength(0.8)
    , lastNoteOnTimestamp(0)
{
    // Initialize default pattern
    currentPattern.length = 16;
    currentPattern.tempo = tempo;
    currentPattern.gridSize = 0.25; // 16th notes
}

GrooveSequencerAudioProcessor::~GrooveSequencerAudioProcessor()
{
    stopAllNotes();
}

void GrooveSequencerAudioProcessor::prepareToPlay(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;
    currentStep = 0;
    lastNoteOnTimestamp = 0;
    stopAllNotes();
}

void GrooveSequencerAudioProcessor::releaseResources()
{
    stopAllNotes();
}

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    
    if (!playing) return;
    
    auto* playHead = getPlayHead();
    if (playHead != nullptr)
    {
        auto positionInfo = playHead->getPosition();
        if (positionInfo.hasValue())
        {
            updatePlaybackPosition(positionInfo.get());
        }
    }
    
    processNextStep();
    
    // Process any incoming MIDI
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            activeNotes.push_back(msg.getNoteNumber());
        }
        else if (msg.isNoteOff())
        {
            auto it = std::find(activeNotes.begin(), activeNotes.end(), msg.getNoteNumber());
            if (it != activeNotes.end())
                activeNotes.erase(it);
        }
    }
}

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::AudioBuffer<float> floatBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        floatBuffer.copyFrom(ch, 0, buffer.getReadPointer(ch), buffer.getNumSamples());
    }
    
    processBlock(floatBuffer, midiMessages);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        buffer.copyFrom(ch, 0, floatBuffer.getReadPointer(ch), buffer.getNumSamples());
    }
}

void GrooveSequencerAudioProcessor::updatePlaybackPosition(juce::AudioPlayHead::PositionInfo& pos)
{
    const juce::ScopedLock sl(playbackLock);
    
    if (pos.getBpm().hasValue())
        tempo = *pos.getBpm();
        
    if (pos.getPpqPosition().hasValue())
    {
        double ppqPerStep = currentPattern.gridSize;
        double currentPpq = *pos.getPpqPosition();
        
        // Apply swing
        if (currentStep % 2 == 1 && swingAmount > 0.0)
        {
            ppqPerStep *= (1.0 + swingAmount);
        }
        
        int newStep = static_cast<int>(currentPpq / ppqPerStep) % currentPattern.length;
        
        if (newStep != currentStep)
        {
            currentStep = newStep;
            processNextStep();
        }
    }
}

void GrooveSequencerAudioProcessor::processNextStep()
{
    const juce::ScopedLock sl(playbackLock);
    
    // Check if we're within loop points
    if (currentStep < loopStartStep || currentStep > loopEndStep)
    {
        currentStep = loopStartStep;
        return;
    }
    
    // Find notes that start on this step
    for (const auto& note : currentPattern.notes)
    {
        double noteStartStep = note.startTime / currentPattern.gridSize;
        if (std::abs(noteStartStep - currentStep) < 0.001) // Allow for floating point comparison
        {
            triggerNote(note);
        }
    }
}

void GrooveSequencerAudioProcessor::triggerNote(const Note& note)
{
    if (note.isRest) return;
    
    // Calculate actual duration based on gate length and staccato
    double actualDuration = note.duration;
    if (note.isStaccato)
        actualDuration *= 0.5;
    actualDuration *= gateLength;
    
    // Calculate velocity based on accent and velocity scale
    int velocity = note.velocity;
    if (note.accent > 0)
        velocity = juce::jmin(127, static_cast<int>(velocity * (1.0 + note.accent * 0.2)));
    velocity = static_cast<int>(velocity * velocityScale);
    
    // Send note on
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, note.pitch, (uint8)velocity);
    noteOn.setTimeStamp(juce::Time::getMillisecondCounterHiRes());
    lastNoteOnTimestamp = noteOn.getTimeStamp();
    
    // Schedule note off
    double noteOffTime = actualDuration * (60.0 / tempo) * 1000.0; // Convert to milliseconds
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, note.pitch);
    noteOff.setTimeStamp(lastNoteOnTimestamp + noteOffTime);
    
    activeNotes.push_back(note.pitch);
}

void GrooveSequencerAudioProcessor::stopAllNotes()
{
    const juce::ScopedLock sl(playbackLock);
    
    for (int note : activeNotes)
    {
        juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, note);
        noteOff.setTimeStamp(juce::Time::getMillisecondCounterHiRes());
    }
    
    activeNotes.clear();
}

void GrooveSequencerAudioProcessor::startPlayback()
{
    const juce::ScopedLock sl(playbackLock);
    currentStep = loopStartStep;
    playing = true;
}

void GrooveSequencerAudioProcessor::stopPlayback()
{
    const juce::ScopedLock sl(playbackLock);
    playing = false;
    stopAllNotes();
}

void GrooveSequencerAudioProcessor::setTempo(double newTempo)
{
    const juce::ScopedLock sl(playbackLock);
    tempo = newTempo;
    currentPattern.tempo = newTempo;
}

void GrooveSequencerAudioProcessor::setLoopPoints(int startStep, int endStep)
{
    const juce::ScopedLock sl(playbackLock);
    loopStartStep = juce::jlimit(0, currentPattern.length - 1, startStep);
    loopEndStep = juce::jlimit(loopStartStep, currentPattern.length - 1, endStep);
}

void GrooveSequencerAudioProcessor::setPattern(const Pattern& newPattern)
{
    const juce::ScopedLock sl(playbackLock);
    currentPattern = newPattern;
    currentStep = loopStartStep;
}

void GrooveSequencerAudioProcessor::clearPattern()
{
    const juce::ScopedLock sl(playbackLock);
    currentPattern.notes.clear();
    stopAllNotes();
}

void GrooveSequencerAudioProcessor::setGridSize(double size)
{
    const juce::ScopedLock sl(playbackLock);
    currentPattern.gridSize = size;
}

void GrooveSequencerAudioProcessor::setSwingAmount(double amount)
{
    const juce::ScopedLock sl(playbackLock);
    swingAmount = juce::jlimit(0.0, 1.0, amount);
}

void GrooveSequencerAudioProcessor::setVelocityScale(double scale)
{
    const juce::ScopedLock sl(playbackLock);
    velocityScale = juce::jlimit(0.0, 2.0, scale);
}

void GrooveSequencerAudioProcessor::setGateLength(double length)
{
    const juce::ScopedLock sl(playbackLock);
    gateLength = juce::jlimit(0.1, 1.0, length);
}

void GrooveSequencerAudioProcessor::playbackStarted()
{
    startPlayback();
}

void GrooveSequencerAudioProcessor::playbackStopped()
{
    stopPlayback();
}

juce::AudioProcessorEditor* GrooveSequencerAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void GrooveSequencerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("GrooveSequencerState");
    
    // Save pattern data
    juce::ValueTree patternState("Pattern");
    patternState.setProperty("length", currentPattern.length, nullptr);
    patternState.setProperty("tempo", currentPattern.tempo, nullptr);
    patternState.setProperty("gridSize", currentPattern.gridSize, nullptr);
    
    // Save notes
    for (const auto& note : currentPattern.notes)
    {
        juce::ValueTree noteState("Note");
        noteState.setProperty("pitch", note.pitch, nullptr);
        noteState.setProperty("startTime", note.startTime, nullptr);
        noteState.setProperty("duration", note.duration, nullptr);
        noteState.setProperty("velocity", note.velocity, nullptr);
        noteState.setProperty("isRest", note.isRest, nullptr);
        noteState.setProperty("isStaccato", note.isStaccato, nullptr);
        noteState.setProperty("accent", note.accent, nullptr);
        patternState.addChild(noteState, -1, nullptr);
    }
    
    state.addChild(patternState, -1, nullptr);
    
    // Save playback parameters
    state.setProperty("loopStartStep", loopStartStep, nullptr);
    state.setProperty("loopEndStep", loopEndStep, nullptr);
    state.setProperty("swingAmount", swingAmount, nullptr);
    state.setProperty("velocityScale", velocityScale, nullptr);
    state.setProperty("gateLength", gateLength, nullptr);
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GrooveSequencerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr)
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xml);
        
        if (juce::ValueTree patternState = state.getChildWithName("Pattern"))
        {
            Pattern newPattern;
            newPattern.length = patternState.getProperty("length", 16);
            newPattern.tempo = patternState.getProperty("tempo", 120.0);
            newPattern.gridSize = patternState.getProperty("gridSize", 0.25);
            
            for (int i = 0; i < patternState.getNumChildren(); ++i)
            {
                if (juce::ValueTree noteState = patternState.getChild(i))
                {
                    Note note;
                    note.pitch = noteState.getProperty("pitch", 60);
                    note.startTime = noteState.getProperty("startTime", 0.0);
                    note.duration = noteState.getProperty("duration", 0.25);
                    note.velocity = noteState.getProperty("velocity", 100);
                    note.isRest = noteState.getProperty("isRest", false);
                    note.isStaccato = noteState.getProperty("isStaccato", false);
                    note.accent = noteState.getProperty("accent", 0);
                    newPattern.notes.push_back(note);
                }
            }
            
            setPattern(newPattern);
        }
        
        // Load playback parameters
        loopStartStep = state.getProperty("loopStartStep", 0);
        loopEndStep = state.getProperty("loopEndStep", 15);
        swingAmount = state.getProperty("swingAmount", 0.0);
        velocityScale = state.getProperty("velocityScale", 1.0);
        gateLength = state.getProperty("gateLength", 0.8);
    }
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveSequencerAudioProcessor();
} 