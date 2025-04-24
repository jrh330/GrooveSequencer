#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdint>

GrooveSequencerAudioProcessor::GrooveSequencerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Initialize default pattern
    currentPattern.length = 16;
    currentPattern.tempo = 120.0;
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

bool GrooveSequencerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input layout must match output layout
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear output buffers
    buffer.clear();
    midiMessages.clear();
    
    // Get playhead position
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            updatePlaybackPosition(*positionInfo);
        }
    }
    
    // Process MIDI
    for (const auto metadata : midiMessages)
    {
        handleIncomingMidiMessage(metadata.getMessage());
    }
    
    // Update pattern playback
    if (playing)
    {
        processNextStep();
    }
}

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    // Convert double buffer to float for processing
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Ensure float buffer is large enough
    floatBuffer.setSize(numChannels, numSamples, false, false, true);
    
    // Copy double buffer to float buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* source = buffer.getReadPointer(ch);
        auto* dest = floatBuffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            dest[i] = static_cast<float>(source[i]);
    }
    
    // Process the float buffer
    processBlock(floatBuffer, midiMessages);
    
    // Copy back to double buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* source = floatBuffer.getReadPointer(ch);
        auto* dest = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            dest[i] = source[i];
    }
}

void GrooveSequencerAudioProcessor::updatePlaybackPosition(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
    if (syncToHost)
    {
        if (auto bpm = positionInfo.getBpm())
            setTempo(*bpm);
        
        if (auto ppqPosition = positionInfo.getPpqPosition())
            currentPosition = *ppqPosition;
    }
}

void GrooveSequencerAudioProcessor::handleIncomingMidiMessage(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        handleNoteOn(message);
    }
    else if (message.isNoteOff())
    {
        handleNoteOff(message);
    }
}

void GrooveSequencerAudioProcessor::handleNoteOn(const juce::MidiMessage& noteOn)
{
    lastNoteOnTimestamp = static_cast<juce::uint32>(noteOn.getTimeStamp());
    // TODO: Implement note triggering
}

void GrooveSequencerAudioProcessor::handleNoteOff(const juce::MidiMessage& /*noteOff*/)
{
    // TODO: Implement note off handling
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
    
    // Move to next step
    currentStep++;
    if (currentStep > loopEndStep)
        currentStep = loopStartStep;
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
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, note.pitch, static_cast<float>(velocity));
    noteOn.setTimeStamp(juce::Time::getMillisecondCounterHiRes());
    lastNoteOnTimestamp = static_cast<juce::uint32>(noteOn.getTimeStamp());
    
    // Schedule note off
    double noteOffTime = actualDuration * (60.0 / currentPattern.tempo) * 1000.0; // Convert to milliseconds
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
    juce::ScopedLock sl(playbackLock);
    swingAmount = juce::jlimit(0.0, 1.0, amount);
}

void GrooveSequencerAudioProcessor::setVelocityScale(double scale)
{
    juce::ScopedLock sl(playbackLock);
    velocityScale = juce::jlimit(0.0, 2.0, scale);
}

void GrooveSequencerAudioProcessor::setGateLength(double length)
{
    juce::ScopedLock sl(playbackLock);
    gateLength = juce::jlimit(0.1, 1.0, length);
}

void GrooveSequencerAudioProcessor::playbackStarted()
{
    juce::ScopedLock sl(playbackLock);
    currentStep = 0;
    activeNotes.clear();
}

void GrooveSequencerAudioProcessor::playbackStopped()
{
    juce::ScopedLock sl(playbackLock);
    currentStep = 0;
    // Send note-offs for any active notes
    juce::MidiBuffer tempMidiBuffer;
    for (int note : activeNotes)
    {
        tempMidiBuffer.addEvent(juce::MidiMessage::noteOff(1, note, 0.0f), 0);
    }
    activeNotes.clear();
}

juce::AudioProcessorEditor* GrooveSequencerAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void GrooveSequencerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    state = juce::ValueTree("GrooveSequencer");
    state.setProperty("swingAmount", swingAmount, nullptr);
    state.setProperty("velocityScale", velocityScale, nullptr);
    state.setProperty("gateLength", gateLength, nullptr);
    state.setProperty("syncToHost", syncToHost, nullptr);
    state.setProperty("articulationStyle", static_cast<int>(articulationStyle), nullptr);
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GrooveSequencerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        state = juce::ValueTree::fromXml(*xmlState);
        
        swingAmount = state.getProperty("swingAmount", 0.0);
        velocityScale = state.getProperty("velocityScale", 1.0);
        gateLength = state.getProperty("gateLength", 0.8);
        syncToHost = state.getProperty("syncToHost", false);
        int artStyle = state.getProperty("articulationStyle", 0);
        articulationStyle = static_cast<ArticulationStyle>(artStyle);
    }
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveSequencerAudioProcessor();
} 