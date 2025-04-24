#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Components/GridSequencerComponent.h"
#include "GrooveSequencerLookAndFeel.h"

//==============================================================================
GrooveSequencerAudioProcessorEditor::GrooveSequencerAudioProcessorEditor(GrooveSequencerAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
    , lookAndFeel(std::make_unique<GrooveSequencerLookAndFeel>())
    , gridSequencer(std::make_unique<GridSequencerComponent>(p))
{
    setLookAndFeel(lookAndFeel.get());
    addAndMakeVisible(*gridSequencer);
    
    // Set up transport controls
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(tempoSlider);
    addAndMakeVisible(tempoLabel);
    
    // Set up grid controls
    addAndMakeVisible(swingSlider);
    addAndMakeVisible(swingLabel);
    addAndMakeVisible(velocityScaleSlider);
    addAndMakeVisible(velocityScaleLabel);
    addAndMakeVisible(gateLengthSlider);
    addAndMakeVisible(gateLengthLabel);
    
    // Configure sliders
    tempoSlider.setRange(40.0, 240.0, 1.0);
    tempoSlider.setValue(120.0);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    
    swingSlider.setRange(0.0, 1.0, 0.01);
    swingSlider.setValue(0.0);
    swingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    
    velocityScaleSlider.setRange(0.0, 2.0, 0.01);
    velocityScaleSlider.setValue(1.0);
    velocityScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    
    gateLengthSlider.setRange(0.1, 1.0, 0.01);
    gateLengthSlider.setValue(0.8);
    gateLengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    
    // Set up labels
    tempoLabel.setText("Tempo", juce::dontSendNotification);
    swingLabel.setText("Swing", juce::dontSendNotification);
    velocityScaleLabel.setText("Velocity", juce::dontSendNotification);
    gateLengthLabel.setText("Gate", juce::dontSendNotification);
    
    // Set up button callbacks
    playButton.onClick = [this]() { audioProcessor.startPlayback(); };
    stopButton.onClick = [this]() { audioProcessor.stopPlayback(); };
    
    // Set up slider callbacks
    tempoSlider.onValueChange = [this]() { audioProcessor.setTempo(tempoSlider.getValue()); };
    swingSlider.onValueChange = [this]() { audioProcessor.setSwingAmount(swingSlider.getValue()); };
    velocityScaleSlider.onValueChange = [this]() { audioProcessor.setVelocityScale(velocityScaleSlider.getValue()); };
    gateLengthSlider.onValueChange = [this]() { audioProcessor.setGateLength(gateLengthSlider.getValue()); };
    
    // Set initial size
    setSize(800, 600);
}

GrooveSequencerAudioProcessorEditor::~GrooveSequencerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GrooveSequencerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GrooveSequencerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Top section for transport and controls
    auto topSection = area.removeFromTop(100);
    auto transportSection = topSection.removeFromLeft(200);
    
    // Transport controls
    playButton.setBounds(transportSection.removeFromTop(40).reduced(5));
    stopButton.setBounds(transportSection.removeFromTop(40).reduced(5));
    
    // Control sliders
    auto sliderWidth = (topSection.getWidth() - 20) / 4;
    
    auto tempoArea = topSection.removeFromLeft(sliderWidth);
    tempoLabel.setBounds(tempoArea.removeFromTop(20));
    tempoSlider.setBounds(tempoArea.reduced(5));
    
    auto swingArea = topSection.removeFromLeft(sliderWidth);
    swingLabel.setBounds(swingArea.removeFromTop(20));
    swingSlider.setBounds(swingArea.reduced(5));
    
    auto velocityArea = topSection.removeFromLeft(sliderWidth);
    velocityScaleLabel.setBounds(velocityArea.removeFromTop(20));
    velocityScaleSlider.setBounds(velocityArea.reduced(5));
    
    auto gateArea = topSection;
    gateLengthLabel.setBounds(gateArea.removeFromTop(20));
    gateLengthSlider.setBounds(gateArea.reduced(5));
    
    // Grid sequencer takes up the rest of the space
    gridSequencer->setBounds(area);
} 