#pragma once

#include <JuceHeader.h>

class TransportComponent : public juce::Component,
                         public juce::Timer
{
public:
    TransportComponent();
    ~TransportComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    // Transport controls
    void startPlayback();
    void stopPlayback();
    void setTempo(double newTempo);
    void setLoopPoints(int startStep, int endStep);
    
    // Callbacks
    std::function<void()> onPlaybackStarted;
    std::function<void()> onPlaybackStopped;
    std::function<void(double)> onTempoChanged;
    std::function<void(int, int)> onLoopPointsChanged;

private:
    // UI Components
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::Slider> tempoSlider;
    std::unique_ptr<juce::Label> tempoLabel;
    std::unique_ptr<juce::Slider> loopStartSlider;
    std::unique_ptr<juce::Slider> loopEndSlider;
    
    // State
    bool isPlaying;
    double tempo;
    int loopStart;
    int loopEnd;
    
    // Helper methods
    void initializeButtons();
    void initializeTempoControl();
    void initializeLoopControls();
    
    void handlePlayButton();
    void handleStopButton();
    void handleTempoChange();
    void handleLoopPointsChange();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportComponent)
}; 