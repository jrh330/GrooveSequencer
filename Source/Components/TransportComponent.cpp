#include "TransportComponent.h"

TransportComponent::TransportComponent()
    : isPlaying(false)
    , tempo(120.0)
    , loopStart(0)
    , loopEnd(16)
{
    // Initialize UI components
    playButton = std::make_unique<juce::TextButton>("Play");
    stopButton = std::make_unique<juce::TextButton>("Stop");
    tempoSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    tempoLabel = std::make_unique<juce::Label>("", "Tempo");
    loopStartSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    loopEndSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    
    // Add components
    addAndMakeVisible(playButton.get());
    addAndMakeVisible(stopButton.get());
    addAndMakeVisible(tempoSlider.get());
    addAndMakeVisible(tempoLabel.get());
    addAndMakeVisible(loopStartSlider.get());
    addAndMakeVisible(loopEndSlider.get());
    
    // Initialize controls
    initializeButtons();
    initializeTempoControl();
    initializeLoopControls();
    
    startTimerHz(30); // 30fps refresh rate
}

TransportComponent::~TransportComponent()
{
    stopTimer();
}

void TransportComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    
    // Draw labels
    g.drawText("Loop Start:", 10, 70, 80, 20, juce::Justification::left);
    g.drawText("Loop End:", 10, 100, 80, 20, juce::Justification::left);
}

void TransportComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    int controlHeight = 25;
    int buttonWidth = 80;
    
    // Position transport controls
    playButton->setBounds(10, 10, buttonWidth, controlHeight);
    stopButton->setBounds(buttonWidth + 20, 10, buttonWidth, controlHeight);
    
    // Position tempo control
    tempoLabel->setBounds(10, 40, 80, controlHeight);
    tempoSlider->setBounds(90, 40, bounds.getWidth() - 100, controlHeight);
    
    // Position loop controls
    loopStartSlider->setBounds(90, 70, bounds.getWidth() - 100, controlHeight);
    loopEndSlider->setBounds(90, 100, bounds.getWidth() - 100, controlHeight);
}

void TransportComponent::timerCallback()
{
    // Update UI based on playback state
    if (isPlaying)
    {
        playButton->setToggleState(true, juce::dontSendNotification);
        stopButton->setEnabled(true);
    }
    else
    {
        playButton->setToggleState(false, juce::dontSendNotification);
        stopButton->setEnabled(false);
    }
}

void TransportComponent::startPlayback()
{
    isPlaying = true;
    if (onPlaybackStarted)
        onPlaybackStarted();
}

void TransportComponent::stopPlayback()
{
    isPlaying = false;
    if (onPlaybackStopped)
        onPlaybackStopped();
}

void TransportComponent::setTempo(double newTempo)
{
    tempo = newTempo;
    tempoSlider->setValue(tempo, juce::dontSendNotification);
}

void TransportComponent::setLoopPoints(int startStep, int endStep)
{
    loopStart = startStep;
    loopEnd = endStep;
    
    loopStartSlider->setValue(loopStart, juce::dontSendNotification);
    loopEndSlider->setValue(loopEnd, juce::dontSendNotification);
}

void TransportComponent::initializeButtons()
{
    playButton->setToggleable(true);
    playButton->onClick = [this] { handlePlayButton(); };
    
    stopButton->setEnabled(false);
    stopButton->onClick = [this] { handleStopButton(); };
}

void TransportComponent::initializeTempoControl()
{
    tempoSlider->setRange(40.0, 240.0, 1.0);
    tempoSlider->setValue(tempo);
    tempoSlider->setTextValueSuffix(" BPM");
    tempoSlider->onValueChange = [this] { handleTempoChange(); };
    
    tempoLabel->setJustificationType(juce::Justification::right);
}

void TransportComponent::initializeLoopControls()
{
    loopStartSlider->setRange(0, 15, 1);
    loopStartSlider->setValue(loopStart);
    loopStartSlider->onValueChange = [this] { handleLoopPointsChange(); };
    
    loopEndSlider->setRange(1, 16, 1);
    loopEndSlider->setValue(loopEnd);
    loopEndSlider->onValueChange = [this] { handleLoopPointsChange(); };
}

void TransportComponent::handlePlayButton()
{
    if (playButton->getToggleState())
        startPlayback();
    else
        stopPlayback();
}

void TransportComponent::handleStopButton()
{
    stopPlayback();
    playButton->setToggleState(false, juce::dontSendNotification);
}

void TransportComponent::handleTempoChange()
{
    tempo = tempoSlider->getValue();
    if (onTempoChanged)
        onTempoChanged(tempo);
}

void TransportComponent::handleLoopPointsChange()
{
    loopStart = static_cast<int>(loopStartSlider->getValue());
    loopEnd = static_cast<int>(loopEndSlider->getValue());
    
    // Ensure loop end is always after loop start
    if (loopEnd <= loopStart)
    {
        loopEnd = loopStart + 1;
        loopEndSlider->setValue(loopEnd, juce::dontSendNotification);
    }
    
    if (onLoopPointsChanged)
        onLoopPointsChanged(loopStart, loopEnd);
} 