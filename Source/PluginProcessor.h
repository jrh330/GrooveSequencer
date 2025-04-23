#pragma once

#include <JuceHeader.h>
#include "PatternTransformer.h"

class GrooveSequencerAudioProcessor : public juce::AudioProcessor
{
public:
    GrooveSequencerAudioProcessor();
    ~GrooveSequencerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Pattern management
    void setPattern(const Pattern& newPattern);
    Pattern getCurrentPattern() const { return currentPattern; }
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