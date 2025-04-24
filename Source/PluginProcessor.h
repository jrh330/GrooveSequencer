#pragma once

#include <JuceHeader.h>
#include "Pattern.h"
#include "PatternTransformer.h"

class GrooveSequencerAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener
{
public:
    GrooveSequencerAudioProcessor();
    ~GrooveSequencerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

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
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Pattern management
    void setPattern(const Pattern& pattern);
    Pattern& getPattern() { return currentPattern; }
    const Pattern& getPattern() const { return currentPattern; }
    
    // Playback state
    void setLoopMode(bool shouldLoop) { loopMode = shouldLoop; }
    bool isLooping() const { return loopMode; }
    void setPlaying(bool shouldPlay) { 
        playing = shouldPlay; 
        if (!playing) stopAllNotes();
    }
    bool isPlaying() const { return playing; }
    void resetPlayhead() { currentPosition = 0; }

    // Parameter state
    juce::AudioProcessorValueTreeState& getState() { return state; }

    // Pattern transformation
    void setTransformationType(TransformationType type);
    void setRhythmPattern(RhythmPattern pattern);
    void setArticulationStyle(ArticulationStyle style);
    
    // Pattern generation
    void generateNewPattern(int length);
    void transformCurrentPattern();
    
    // Pattern state
    bool isPatternModified() const { return patternModified; }
    void clearModifiedFlag() { patternModified = false; }

    // Playback control
    void startPlayback();
    void stopPlayback();
    void stopAllNotes();
    
    // Parameter control
    void setTempo(double newTempo);
    void setSwingAmount(double amount);
    void setVelocityScale(double scale);
    void setGateLength(double length);

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void handleIncomingMidiMessage(const juce::MidiMessage& message);

    // Grid control
    void updateGridCell(int row, int col, bool active, float velocity, int accent, bool isStaccato);
    int getCurrentStep() const { return currentStep; }
    void setGridSize(double size) { currentGridSize = size; }
    double getGridSize() const { return currentGridSize; }

private:
    void updatePlaybackPosition(int numSamples);
    void sendNoteEvents();
    
    Pattern currentPattern;
    PatternTransformer transformer;
    
    bool loopMode{true};
    bool playing{false};
    double currentPosition{0.0};
    double sampleRate{44100.0};
    int samplesPerBeat{0};
    int currentStep{-1};  // Initialize to -1 to ensure first step triggers
    double currentGridSize{0.25};  // Default to 16th notes
    
    juce::AudioProcessorValueTreeState state;
    std::atomic<float>* tempoParam{nullptr};
    std::atomic<float>* gridSizeParam{nullptr};
    std::atomic<float>* lengthParam{nullptr};

    bool patternModified{false};
    
    // Additional parameters
    double swingAmount{0.0};
    double velocityScale{1.0};
    double gateLength{0.5};
    
    juce::MidiBuffer midiBuffer;
    juce::CriticalSection patternLock;

    juce::AudioBuffer<float> floatBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessor)
}; 