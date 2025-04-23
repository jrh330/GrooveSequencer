#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PatternTransformer.h"

struct Note {
    int pitch;
    int velocity;
    double startTime;
    double duration;
    bool isStaccato;
    int accent;
};

struct Pattern {
    double tempo;
    int length;
    double gridSize;
    std::vector<Note> notes;
};

class GrooveSequencerAudioProcessor : public juce::AudioProcessor
{
public:
    GrooveSequencerAudioProcessor();
    ~GrooveSequencerAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int /*index*/) override {}
    const juce::String getProgramName(int /*index*/) override { return {}; }
    void changeProgramName(int /*index*/, const juce::String& /*newName*/) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    Pattern& getCurrentPattern() { return currentPattern; }
    void setCurrentPattern(const Pattern& pattern) { currentPattern = pattern; }

    // Pattern management
    void startPlayback();
    void stopPlayback();
    void setTempo(double newTempo);
    
    // Pattern transformation
    void applyTransformation(TransformationType type);
    void setRhythmPattern(RhythmPattern pattern);
    void setArticulation(ArticulationStyle style);
    
    // MIDI handling
    void handleIncomingMidiMessage(const juce::MidiMessage& message);
    void addMidiEventToBuffer(const juce::MidiMessage& message, int samplePosition);

private:
    PatternTransformer transformer;
    Pattern currentPattern;
    double currentSampleRate;
    
    double tempo;
    bool isPlaying;
    double currentBeat;
    double samplesPerBeat;
    
    juce::MidiBuffer midiBuffer;
    std::vector<Note> scheduledNotes;
    
    void updateScheduledNotes();
    void processNextBlock(juce::MidiBuffer& midiMessages, int numSamples);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessor)
}; 