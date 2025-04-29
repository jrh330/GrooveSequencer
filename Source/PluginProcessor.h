#pragma once

#include <JuceHeader.h>
#include "Pattern.h"
#include "PatternTransformer.h"
#include "Common.h"

namespace Parameters
{
    // Parameter IDs
    static const juce::String TEMPO_ID = "tempo";
    static const juce::String GRID_SIZE_ID = "gridSize";
    static const juce::String LENGTH_ID = "length";
    static const juce::String SWING_ID = "swing";
    static const juce::String VELOCITY_ID = "velocity";
    static const juce::String GATE_ID = "gate";
    
    // Parameter Defaults
    static constexpr float DEFAULT_TEMPO = 120.0f;
    static constexpr float DEFAULT_GRID_SIZE = 16.0f;
    static constexpr float DEFAULT_LENGTH = 16.0f;
    static constexpr float DEFAULT_SWING = 0.0f;
    static constexpr float DEFAULT_VELOCITY = 1.0f;
    static constexpr float DEFAULT_GATE = 0.5f;
    
    // Create parameter layout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            TEMPO_ID, "Tempo", 30.0f, 300.0f, DEFAULT_TEMPO));
            
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            GRID_SIZE_ID, "Grid Size", 1, 32, DEFAULT_GRID_SIZE));
            
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            LENGTH_ID, "Length", 1, 64, DEFAULT_LENGTH));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            SWING_ID, "Swing", 0.0f, 1.0f, DEFAULT_SWING));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            VELOCITY_ID, "Velocity", 0.0f, 2.0f, DEFAULT_VELOCITY));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            GATE_ID, "Gate", 0.1f, 1.0f, DEFAULT_GATE));
            
        return { params.begin(), params.end() };
    }
}

class SineVoice {
public:
    SineVoice() : currentFrequency(0.0f), phase(0.0f), amplitude(0.0f), sampleRate(44100.0f), isPlaying(false), currentNote(-1) {}
    
    void startNote(float frequency, float velocity) {
        currentFrequency = frequency;
        amplitude = velocity;
        phase = 0.0f;
        isPlaying = true;
        currentNote = juce::MidiMessage::getMidiNoteInHertz(frequency);
    }
    
    void stopNote() {
        amplitude = 0.0f;
        currentFrequency = 0.0f;
        isPlaying = false;
        currentNote = -1;
    }
    
    float getNextSample() {
        if (amplitude <= 0.0f) return 0.0f;
        
        float sample = std::sin(phase * 2.0f * juce::MathConstants<float>::pi) * amplitude;
        phase += currentFrequency / sampleRate;
        while (phase >= 1.0f) phase -= 1.0f;
        
        return sample;
    }
    
    void setSampleRate(float newSampleRate) {
        sampleRate = newSampleRate;
    }

    bool isActive() const { return isPlaying; }
    float getCurrentNote() const { return currentNote; }
    
private:
    float currentFrequency;
    float phase;
    float amplitude;
    float sampleRate;
    bool isPlaying;
    float currentNote;
};

class GrooveSequencerAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener,
                                    private juce::Timer
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
    void setCurrentProgram(int /*index*/) override {}
    const juce::String getProgramName(int /*index*/) override { return {}; }
    void changeProgramName(int /*index*/, const juce::String& /*newName*/) override {}

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
    juce::String getTransformationTypeString(TransformationType type) const;
    
    // Pattern generation
    void generateNewPattern();
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
    double getTempo() const;
    void setSwingAmount(double amount);
    void setVelocityScale(double scale);
    void setGateLength(double length);
    void setLength(int newLength);
    int getLength() const { return static_cast<int>(currentPattern.getNotes().size()); }

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void handleMidiInput(const juce::MidiMessage& message);

    // Grid control
    void updateGridCell(int row, int col, bool active, float velocity, int accent, bool isStaccato);
    int getCurrentStep() const { return currentStep; }
    void setGridSize(double size) { currentGridSize = size; }
    double getGridSize() const { return currentGridSize; }

    // Recording control
    bool isCurrentlyRecording() const;

    // Pattern storage
    void savePattern(const juce::File& file);
    void loadPattern(const juce::File& file);

    // Note division control
    void setNoteDivision(NoteDivision newDivision) { 
        division = newDivision;
        fileLogger->logMessage("Note division set to: " + EnumToString::toString(division));
    }
    NoteDivision getNoteDivision() const { return division; }

protected:
    void timerCallback() override;

private:
    void updatePlaybackPosition(int numSamples);
    void triggerNotesForCurrentStep();
    void sendNoteEvents();

    juce::AudioProcessorValueTreeState state;
    Pattern currentPattern;
    PatternTransformer transformer;
    
    bool loopMode;
    bool playing;
    double currentPosition;
    double sampleRate;
    double samplesPerBeat;
    int currentStep;
    double currentGridSize;
    double swingAmount;
    double velocityScale;
    double gateLength;
    bool patternModified;
    NoteDivision division;
    bool isRecording;
    
    TransformationType transformationType;
    RhythmPattern rhythmPattern;
    ArticulationStyle articulationStyle;
    
    juce::CriticalSection patternLock;
    juce::MidiBuffer midiBuffer;
    juce::AudioBuffer<float> floatBuffer;

    // Logger
    std::unique_ptr<juce::FileLogger> fileLogger;

    // Synth voices
    static constexpr int kNumVoices = 16;
    std::array<SineVoice, kNumVoices> voices;
    
    // Find a free voice or steal the oldest one
    SineVoice* findFreeVoice() {
        for (auto& voice : voices) {
            if (!voice.isActive()) return &voice;
        }
        return &voices[0]; // If no free voice, steal the first one
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessor)
};