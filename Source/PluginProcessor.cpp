#include "PluginProcessor.h"
#include "PluginEditor.h"

GrooveSequencerAudioProcessor::GrooveSequencerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , tempo(120.0)
    , isPlaying(false)
    , currentBeat(0.0)
    , samplesPerBeat(0.0)
{
    // Initialize with a basic pattern
    currentPattern.gridSize = 0.25; // 16th notes
    currentPattern.tempo = tempo;
}

GrooveSequencerAudioProcessor::~GrooveSequencerAudioProcessor()
{
}

void GrooveSequencerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
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

void GrooveSequencerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    midiMessages.clear();
    
    if (!isPlaying)
        return;
        
    const int numSamples = buffer.getNumSamples();
    
    // Process MIDI events for this block
    processNextBlock(midiMessages, numSamples);
    
    // Update beat position
    currentBeat += numSamples / samplesPerBeat;
    while (currentBeat >= currentPattern.length)
        currentBeat -= currentPattern.length;
}

bool GrooveSequencerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Accept mono or stereo for input and output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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
            
            auto noteOn = juce::MidiMessage::noteOn(1, note.pitch, (uint8)velocity);
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