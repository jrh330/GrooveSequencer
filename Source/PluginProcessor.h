#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>

// Basic note structure for pattern storage
struct Note {
    int pitch;          // MIDI note number
    double startTime;   // Start time in beats
    double duration;    // Duration in beats
    int velocity;       // MIDI velocity (0-127)
    bool isRest;       // Whether this is a rest
    bool isStaccato;   // Staccato articulation flag
    int accent;        // Accent level (0 = none, 1 = medium, 2 = strong)
};

// Pattern structure to hold sequence data
struct Pattern {
    std::vector<Note> notes;
    int length;         // Pattern length in steps
    double tempo;       // Tempo in BPM
    double gridSize;    // Grid size in beats (e.g., 0.25 for 16th notes)
};

class GrooveSequencerAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioPlayHead::Listener
{
public:
    GrooveSequencerAudioProcessor();
    ~GrooveSequencerAudioProcessor() override;

    // Standard JUCE processor methods
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    
    // Plugin properties
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    // Program handling
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    
    // State handling
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Transport control
    void playbackStarted() override;
    void playbackStopped() override;
    void startPlayback();
    void stopPlayback();
    void setTempo(double newTempo);
    void setLoopPoints(int startStep, int endStep);
    bool isPlaying() const { return playing; }
    
    // Pattern management
    void setPattern(const Pattern& newPattern);
    Pattern& getCurrentPattern() { return currentPattern; }
    void clearPattern();
    
    // Real-time parameter handling
    void setGridSize(double size);
    void setSwingAmount(double amount);
    void setVelocityScale(double scale);
    void setGateLength(double length);

private:
    // Pattern playback
    void updatePlaybackPosition(juce::AudioPlayHead::PositionInfo& pos);
    void processNextStep();
    void triggerNote(const Note& note);
    void stopAllNotes();
    
    // Pattern data
    Pattern currentPattern;
    int currentStep;
    bool playing;
    
    // Playback parameters
    double sampleRate;
    double tempo;
    int loopStartStep;
    int loopEndStep;
    double swingAmount;
    double velocityScale;
    double gateLength;
    
    // MIDI state tracking
    std::vector<int> activeNotes;
    juce::uint32 lastNoteOnTimestamp;
    
    // Threading protection
    juce::CriticalSection playbackLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrooveSequencerAudioProcessor)
}; 