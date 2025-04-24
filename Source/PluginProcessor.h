#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include "Pattern.h"
#include "PatternTransformer.h"

class GrooveSequencerAudioProcessor : public juce::AudioProcessor
{
public:
    GrooveSequencerAudioProcessor();
    ~GrooveSequencerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
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
    const Pattern& getPattern() const { return currentPattern; }
    void clearPattern();
    
    // Transport control
    void startPlayback();
    void stopPlayback();
    void setTempo(double newTempo);
    double getTempo() const { return currentPattern.tempo; }
    bool isPlaying() const { return playing; }
    
    // Grid settings
    void setGridSize(double size);
    double getGridSize() const { return currentPattern.gridSize; }
    
    // Playback settings
    void setSyncToHost(bool sync) { syncToHost = sync; }
    bool getSyncToHost() const { return syncToHost; }
    void setSwingAmount(double amount);
    double getSwingAmount() const { return swingAmount; }
    void setVelocityScale(double scale);
    double getVelocityScale() const { return velocityScale; }
    void setGateLength(double length);
    double getGateLength() const { return gateLength; }
    
    void setLoopPoints(int startStep, int endStep);
    
    enum class ArticulationStyle
    {
        Natural,
        Staccato,
        Legato
    };
    
    void setArticulationStyle(ArticulationStyle style) { articulationStyle = style; }
    ArticulationStyle getArticulationStyle() const { return articulationStyle; }

    void playbackStarted();
    void playbackStopped();

private:
    void updatePlaybackPosition(const juce::AudioPlayHead::PositionInfo& positionInfo);
    void handleIncomingMidiMessage(const juce::MidiMessage& message);
    void handleNoteOn(const juce::MidiMessage& noteOn);
    void handleNoteOff(const juce::MidiMessage& noteOff);
    void processNextStep();
    void triggerNote(const Note& note);
    void stopAllNotes();
    
    Pattern currentPattern;
    PatternTransformer transformer;
    
    bool playing{false};
    bool syncToHost{false};
    double currentPosition{0.0};
    double sampleRate{44100.0};
    int currentStep{0};
    int loopStartStep{0};
    int loopEndStep{15};
    
    double swingAmount{0.0};
    double velocityScale{1.0};
    double gateLength{0.8};
    ArticulationStyle articulationStyle{ArticulationStyle::Natural};
    
    juce::uint32 lastNoteOnTimestamp{0};
    juce::AudioBuffer<float> floatBuffer;
    std::vector<int> activeNotes;
    juce::CriticalSection playbackLock;
    
    juce::ValueTree state;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSequencerAudioProcessor)
}; 