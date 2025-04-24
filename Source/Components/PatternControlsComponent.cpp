#include "PatternControlsComponent.h"

PatternControlsComponent::PatternControlsComponent()
{
    // Initialize UI components
    transformationSelector = std::make_unique<juce::ComboBox>("Transformation");
    rhythmPatternSelector = std::make_unique<juce::ComboBox>("Rhythm Pattern");
    articulationStyleSelector = std::make_unique<juce::ComboBox>("Articulation");
    humanizeSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    applyButton = std::make_unique<juce::TextButton>("Apply");
    clearButton = std::make_unique<juce::TextButton>("Clear");
    
    // Add components
    addAndMakeVisible(transformationSelector.get());
    addAndMakeVisible(rhythmPatternSelector.get());
    addAndMakeVisible(articulationStyleSelector.get());
    addAndMakeVisible(humanizeSlider.get());
    addAndMakeVisible(applyButton.get());
    addAndMakeVisible(clearButton.get());
    
    // Initialize selectors
    initializeTransformationSelector();
    initializeRhythmPatternSelector();
    initializeArticulationStyleSelector();
    initializeHumanizeControls();
    initializeButtons();
}

PatternControlsComponent::~PatternControlsComponent()
{
}

void PatternControlsComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColour));
    
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    
    // Draw labels
    g.drawText("Transform:", 10, 10, 100, 20, juce::Justification::left);
    g.drawText("Rhythm:", 10, 40, 100, 20, juce::Justification::left);
    g.drawText("Articulation:", 10, 70, 100, 20, juce::Justification::left);
    g.drawText("Humanize:", 10, 100, 100, 20, juce::Justification::left);
}

void PatternControlsComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    int controlHeight = 25;
    int labelWidth = 100;
    int controlWidth = bounds.getWidth() - labelWidth - 20;
    
    // Position controls
    transformationSelector->setBounds(labelWidth, 10, controlWidth, controlHeight);
    rhythmPatternSelector->setBounds(labelWidth, 40, controlWidth, controlHeight);
    articulationStyleSelector->setBounds(labelWidth, 70, controlWidth, controlHeight);
    humanizeSlider->setBounds(labelWidth, 100, controlWidth, controlHeight);
    
    // Position buttons
    int buttonWidth = (controlWidth - 10) / 2;
    applyButton->setBounds(labelWidth, 130, buttonWidth, controlHeight);
    clearButton->setBounds(labelWidth + buttonWidth + 10, 130, buttonWidth, controlHeight);
}

void PatternControlsComponent::setCurrentPattern(const Pattern& pattern)
{
    currentPattern = pattern;
}

Pattern PatternControlsComponent::getCurrentPattern() const
{
    return currentPattern;
}

void PatternControlsComponent::initializeTransformationSelector()
{
    transformationSelector->addItem("Step Up", static_cast<int>(TransformationType::StepUp) + 1);
    transformationSelector->addItem("Step Down", static_cast<int>(TransformationType::StepDown) + 1);
    transformationSelector->addItem("Up Two Down One", static_cast<int>(TransformationType::UpTwoDownOne) + 1);
    transformationSelector->addItem("Skip One", static_cast<int>(TransformationType::SkipOne) + 1);
    transformationSelector->addItem("Arch", static_cast<int>(TransformationType::Arch) + 1);
    transformationSelector->addItem("Pendulum", static_cast<int>(TransformationType::Pendulum) + 1);
    transformationSelector->addItem("Power Chord", static_cast<int>(TransformationType::PowerChord) + 1);
    transformationSelector->addItem("Random Free", static_cast<int>(TransformationType::RandomFree) + 1);
    transformationSelector->addItem("Random In Key", static_cast<int>(TransformationType::RandomInKey) + 1);
    transformationSelector->addItem("Random Rhythmic", static_cast<int>(TransformationType::RandomRhythmic) + 1);
    transformationSelector->addItem("Invert", static_cast<int>(TransformationType::Invert) + 1);
    transformationSelector->addItem("Mirror", static_cast<int>(TransformationType::Mirror) + 1);
    transformationSelector->addItem("Retrograde", static_cast<int>(TransformationType::Retrograde) + 1);
    
    transformationSelector->onChange = [this] { handleTransformationChange(); };
    transformationSelector->setSelectedItemIndex(0);
}

void PatternControlsComponent::initializeRhythmPatternSelector()
{
    rhythmPatternSelector->addItem("Regular", static_cast<int>(RhythmPattern::Regular) + 1);
    rhythmPatternSelector->addItem("Dotted", static_cast<int>(RhythmPattern::Dotted) + 1);
    rhythmPatternSelector->addItem("Swing", static_cast<int>(RhythmPattern::Swing) + 1);
    rhythmPatternSelector->addItem("Syncopated", static_cast<int>(RhythmPattern::Syncopated) + 1);
    rhythmPatternSelector->addItem("Random", static_cast<int>(RhythmPattern::Random) + 1);
    rhythmPatternSelector->addItem("Clave", static_cast<int>(RhythmPattern::Clave) + 1);
    
    rhythmPatternSelector->onChange = [this] { handleRhythmPatternChange(); };
    rhythmPatternSelector->setSelectedItemIndex(0);
}

void PatternControlsComponent::initializeArticulationStyleSelector()
{
    articulationStyleSelector->addItem("Legato", static_cast<int>(ArticulationStyle::Legato) + 1);
    articulationStyleSelector->addItem("Staccato", static_cast<int>(ArticulationStyle::Staccato) + 1);
    articulationStyleSelector->addItem("Mixed", static_cast<int>(ArticulationStyle::Mixed) + 1);
    articulationStyleSelector->addItem("Accented", static_cast<int>(ArticulationStyle::Accented) + 1);
    articulationStyleSelector->addItem("Random", static_cast<int>(ArticulationStyle::Random) + 1);
    
    articulationStyleSelector->onChange = [this] { handleArticulationStyleChange(); };
    articulationStyleSelector->setSelectedItemIndex(0);
}

void PatternControlsComponent::initializeHumanizeControls()
{
    humanizeSlider->setRange(0.0, 1.0, 0.01);
    humanizeSlider->setValue(0.0);
    humanizeSlider->setTextValueSuffix(" %");
    humanizeSlider->onValueChange = [this] { handleHumanizeChange(); };
}

void PatternControlsComponent::initializeButtons()
{
    applyButton->onClick = [this] { handleApplyButton(); };
    clearButton->onClick = [this] { handleClearButton(); };
}

void PatternControlsComponent::handleTransformationChange()
{
    if (onTransformationSelected)
    {
        auto type = static_cast<TransformationType>(transformationSelector->getSelectedItemIndex());
        onTransformationSelected(type);
    }
}

void PatternControlsComponent::handleRhythmPatternChange()
{
    if (onRhythmPatternSelected)
    {
        auto pattern = static_cast<RhythmPattern>(rhythmPatternSelector->getSelectedItemIndex());
        onRhythmPatternSelected(pattern);
    }
}

void PatternControlsComponent::handleArticulationStyleChange()
{
    if (onArticulationStyleSelected)
    {
        auto style = static_cast<ArticulationStyle>(articulationStyleSelector->getSelectedItemIndex());
        onArticulationStyleSelected(style);
    }
}

void PatternControlsComponent::handleHumanizeChange()
{
    // Handle humanize value changes
    // This will be implemented when we add humanization features
}

void PatternControlsComponent::handleApplyButton()
{
    if (onPatternSelected)
    {
        // Apply current transformations to pattern
        onPatternSelected(currentPattern);
    }
}

void PatternControlsComponent::handleClearButton()
{
    // Clear current pattern
    currentPattern = Pattern();
    if (onPatternSelected)
    {
        onPatternSelected(currentPattern);
    }
} 