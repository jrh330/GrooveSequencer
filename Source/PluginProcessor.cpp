#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdint>

GrooveSequencerAudioProcessor::GrooveSequencerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , currentSampleRate(0.0)
    , currentPattern()
    , tempo(120.0)
    , isPlaying(false)
    , currentBeat(0.0)
    , samplesPerBeat(0.0)
{
}

GrooveSequencerAudioProcessor::~GrooveSequencerAudioProcessor()
{
}

void GrooveSequencerAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    
    // Calculate samples per beat based on tempo
    samplesPerBeat = (sampleRate * 60.0) / tempo;
    
    // Clear any existing MIDI events
    midiBuffer.clear();
    
    // Send program change for Marimba (GM instrument 13)
    juce::MidiMessage programChange = juce::MidiMessage::programChange(1, 12); // 0-based index
    midiBuffer.addEvent(programChange, 0);
}

void GrooveSequencerAudioProcessor::releaseResources()
{
    // Release any resources when playback stops
    midiBuffer.clear();
    scheduledNotes.clear();
}

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    midiMessages.clear();

    // Calculate timing information
    const double localSamplesPerBeat = (60.0 / currentPattern.tempo) * currentSampleRate;
    const double samplesPerGridUnit = localSamplesPerBeat * currentPattern.gridSize;
    
    // Process each note in the pattern
    for (const auto& note : currentPattern.notes) {
        // Calculate sample position for this note
        const int samplePosition = static_cast<int>(note.startTime * samplesPerGridUnit);
        
        if (samplePosition >= 0 && samplePosition < buffer.getNumSamples()) {
            // Calculate velocity with staccato adjustment
            int velocity = static_cast<int>(note.isStaccato ? note.velocity * 0.8 : note.velocity);
            velocity = juce::jlimit(1, 127, velocity + (note.accent * 20)); // Accent affects velocity
            
            // Create MIDI messages for note on and off
            auto noteOn = juce::MidiMessage::noteOn(1, note.pitch, static_cast<uint8_t>(velocity));
            midiMessages.addEvent(noteOn, samplePosition);
            
            // Calculate note off position based on duration
            const int noteOffPosition = static_cast<int>((note.startTime + note.duration) * samplesPerGridUnit);
            if (noteOffPosition < buffer.getNumSamples()) {
                auto noteOff = juce::MidiMessage::noteOff(1, note.pitch);
                midiMessages.addEvent(noteOff, noteOffPosition);
            }
        }
    }
}

bool GrooveSequencerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void GrooveSequencerAudioProcessor::processNextBlock(juce::MidiBuffer& midiMessages, int numSamples)
{
    const double samplesPerGridUnit = samplesPerBeat * currentPattern.gridSize;
    const double startBeat = currentBeat;
    const double endBeat = startBeat + (numSamples / samplesPerBeat);
    
    // Process each note in the pattern
    for (const auto& note : currentPattern.notes)
    {
        double noteStartBeat = note.startTime / currentPattern.gridSize;
        double noteDurationBeats = note.duration / currentPattern.gridSize;
        
        // Check if note should be played in this block
        if (noteStartBeat >= startBeat && noteStartBeat < endBeat)
        {
            // Calculate sample position within the block
            int samplePosition = static_cast<int>((noteStartBeat - startBeat) * samplesPerBeat);
            
            // Create note-on message
            int velocity = note.isStaccato ? note.velocity * 0.8 : note.velocity;
            velocity = juce::jlimit(1, 127, velocity + (note.accent * 20)); // Accent affects velocity
            
            auto noteOn = juce::MidiMessage::noteOn(1, note.pitch, (uint8_t)velocity);
            midiMessages.addEvent(noteOn, samplePosition);
            
            // Calculate note-off time
            double noteLength = note.isStaccato ? noteDurationBeats * 0.5 : noteDurationBeats;
            int noteOffPosition = static_cast<int>(samplePosition + (noteLength * samplesPerBeat));
            
            // If note ends within this block, add note-off message
            if (noteOffPosition < numSamples)
            {
                auto noteOff = juce::MidiMessage::noteOff(1, note.pitch);
                midiMessages.addEvent(noteOff, noteOffPosition);
            }
            else
            {
                // Schedule note-off for later
                scheduledNotes.push_back(note);
            }
        }
    }
    
    // Process any scheduled note-offs
    for (auto it = scheduledNotes.begin(); it != scheduledNotes.end();)
    {
        double noteEndBeat = (it->startTime + it->duration) / currentPattern.gridSize;
        if (noteEndBeat < endBeat)
        {
            int noteOffPosition = static_cast<int>((noteEndBeat - startBeat) * samplesPerBeat);
            if (noteOffPosition >= 0 && noteOffPosition < numSamples)
            {
                auto noteOff = juce::MidiMessage::noteOff(1, it->pitch);
                midiMessages.addEvent(noteOff, noteOffPosition);
            }
            it = scheduledNotes.erase(it);
        }
        else
            ++it;
    }
}

void GrooveSequencerAudioProcessor::setPattern(const Pattern& newPattern)
{
    currentPattern = newPattern;
    updateScheduledNotes();
}

void GrooveSequencerAudioProcessor::startPlayback()
{
    currentBeat = 0.0;
    isPlaying = true;
    updateScheduledNotes();
}

void GrooveSequencerAudioProcessor::stopPlayback()
{
    isPlaying = false;
    midiBuffer.clear();
    scheduledNotes.clear();
    
    // Send all notes off
    for (int channel = 1; channel <= 16; ++channel)
    {
        midiBuffer.addEvent(juce::MidiMessage::allNotesOff(channel), 0);
        midiBuffer.addEvent(juce::MidiMessage::allSoundOff(channel), 0);
    }
}

void GrooveSequencerAudioProcessor::setTempo(double newTempo)
{
    tempo = newTempo;
    if (getSampleRate() > 0)
        samplesPerBeat = (getSampleRate() * 60.0) / tempo;
    currentPattern.tempo = tempo;
}

void GrooveSequencerAudioProcessor::updateScheduledNotes()
{
    scheduledNotes.clear();
}

juce::AudioProcessorEditor* GrooveSequencerAudioProcessor::createEditor()
{
    return new GrooveSequencerAudioProcessorEditor(*this);
}

void GrooveSequencerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Create XML with the plugin's current state
    juce::XmlElement xml("GrooveSequencerState");
    
    // Add pattern data
    auto patternXml = xml.createNewChildElement("Pattern");
    patternXml->setAttribute("tempo", currentPattern.tempo);
    patternXml->setAttribute("length", currentPattern.length);
    
    // Add notes
    for (const auto& note : currentPattern.notes) {
        auto noteXml = patternXml->createNewChildElement("Note");
        noteXml->setAttribute("pitch", note.pitch);
        noteXml->setAttribute("velocity", note.velocity);
        noteXml->setAttribute("startTime", note.startTime);
        noteXml->setAttribute("duration", note.duration);
        noteXml->setAttribute("isStaccato", note.isStaccato);
        noteXml->setAttribute("accent", note.accent);
    }
    
    // Convert XML to binary
    copyXmlToBinary(xml, destData);
}

void GrooveSequencerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Parse binary data back to XML
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    if (xml != nullptr && xml->hasTagName("GrooveSequencerState")) {
        // Get pattern data
        if (auto* patternXml = xml->getChildByName("Pattern")) {
            Pattern newPattern;
            newPattern.tempo = patternXml->getDoubleAttribute("tempo", 120.0);
            newPattern.length = patternXml->getIntAttribute("length", 16);
            
            // Get notes
            newPattern.notes.clear();
            forEachXmlChildElement(*patternXml, noteXml) {
                if (noteXml->hasTagName("Note")) {
                    Note note;
                    note.pitch = noteXml->getIntAttribute("pitch", 60);
                    note.velocity = noteXml->getIntAttribute("velocity", 100);
                    note.startTime = noteXml->getDoubleAttribute("startTime", 0.0);
                    note.duration = noteXml->getDoubleAttribute("duration", 1.0);
                    note.isStaccato = noteXml->getBoolAttribute("isStaccato", false);
                    note.accent = noteXml->getIntAttribute("accent", 0);
                    newPattern.notes.push_back(note);
                }
            }
            
            currentPattern = newPattern;
        }
    }
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveSequencerAudioProcessor();
} 