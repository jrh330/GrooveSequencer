#include "PatternControlsComponent.h"
#include <stdexcept>
#include <sstream>

namespace {
    // Helper function to create detailed error messages
    juce::String createErrorMessage(const char* function, const char* detail, const std::exception& e) {
        std::ostringstream oss;
        oss << "Error in " << function << ": " << detail << " - " << e.what();
        return oss.str();
    }

    // Helper to safely reset a component's state
    template<typename T>
    void safeReset(std::unique_ptr<T>& component, const char* componentName) {
        try {
            if (component) {
                component->setEnabled(false);
                component.reset();
            }
        }
        catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to safely reset " + juce::String(componentName) + ": " + e.what());
        }
    }

    // Helper to safely initialize a combo box
    void safeInitComboBox(juce::ComboBox* box, const char* boxName) {
        if (!box) {
            juce::Logger::writeToLog("Null pointer in safeInitComboBox for " + juce::String(boxName));
            return;
        }
        try {
            box->clear(juce::dontSendNotification);
            box->setEnabled(true);
        }
        catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to initialize " + juce::String(boxName) + ": " + e.what());
            box->setEnabled(false);
        }
    }
}

PatternControlsComponent::PatternControlsComponent()
{
    juce::Logger::writeToLog("Initializing PatternControlsComponent");
    try {
        // Initialize UI components with nullptr checks and detailed error handling
        transformationSelector = std::make_unique<juce::ComboBox>("Transformation");
        rhythmPatternSelector = std::make_unique<juce::ComboBox>("Rhythm Pattern");
        articulationStyleSelector = std::make_unique<juce::ComboBox>("Articulation");
        humanizeSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
        applyButton = std::make_unique<juce::TextButton>("Apply");
        clearButton = std::make_unique<juce::TextButton>("Clear");
        
        if (!transformationSelector || !rhythmPatternSelector || !articulationStyleSelector || 
            !humanizeSlider || !applyButton || !clearButton) {
            throw std::runtime_error("Failed to create one or more UI components");
        }
        
        // Add components with validation and recovery
        addAndMakeVisible(transformationSelector.get());
        addAndMakeVisible(rhythmPatternSelector.get());
        addAndMakeVisible(articulationStyleSelector.get());
        addAndMakeVisible(humanizeSlider.get());
        addAndMakeVisible(applyButton.get());
        addAndMakeVisible(clearButton.get());
        
        // Initialize selectors with validation and recovery
        bool initSuccess = true;
        try {
            initializeTransformationSelector();
        } catch (const std::exception& e) {
            juce::Logger::writeToLog(createErrorMessage("initializeTransformationSelector", "Failed to initialize", e));
            initSuccess = false;
        }
        
        try {
            initializeRhythmPatternSelector();
        } catch (const std::exception& e) {
            juce::Logger::writeToLog(createErrorMessage("initializeRhythmPatternSelector", "Failed to initialize", e));
            initSuccess = false;
        }
        
        try {
            initializeArticulationStyleSelector();
        } catch (const std::exception& e) {
            juce::Logger::writeToLog(createErrorMessage("initializeArticulationStyleSelector", "Failed to initialize", e));
            initSuccess = false;
        }
        
        try {
            initializeHumanizeControls();
        } catch (const std::exception& e) {
            juce::Logger::writeToLog(createErrorMessage("initializeHumanizeControls", "Failed to initialize", e));
            initSuccess = false;
        }
        
        try {
            initializeButtons();
        } catch (const std::exception& e) {
            juce::Logger::writeToLog(createErrorMessage("initializeButtons", "Failed to initialize", e));
            initSuccess = false;
        }

        if (!initSuccess) {
            juce::Logger::writeToLog("Some controls failed to initialize. Component may have limited functionality.");
        }
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog(createErrorMessage("PatternControlsComponent", "Critical initialization failure", e));
        // Clean up any partially initialized components with safe cleanup
        safeReset(transformationSelector, "transformationSelector");
        safeReset(rhythmPatternSelector, "rhythmPatternSelector");
        safeReset(articulationStyleSelector, "articulationStyleSelector");
        safeReset(humanizeSlider, "humanizeSlider");
        safeReset(applyButton, "applyButton");
        safeReset(clearButton, "clearButton");
        throw; // Re-throw to notify parent components
    }
    juce::Logger::writeToLog("PatternControlsComponent initialization completed");
}

PatternControlsComponent::~PatternControlsComponent()
{
}

void PatternControlsComponent::paint(juce::Graphics& g)
{
    if (!isVisible()) return;

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    
    // Draw labels with bounds checking
    auto bounds = getLocalBounds();
    if (bounds.getWidth() > 110 && bounds.getHeight() > 120) {
        g.drawText("Transform:", 10, 10, 100, 20, juce::Justification::left, false);
        g.drawText("Rhythm:", 10, 40, 100, 20, juce::Justification::left, false);
        g.drawText("Articulation:", 10, 70, 100, 20, juce::Justification::left, false);
        g.drawText("Humanize:", 10, 100, 100, 20, juce::Justification::left, false);
    }
}

void PatternControlsComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    if (bounds.isEmpty()) return;

    int controlHeight = 25;
    int labelWidth = 100;
    int controlWidth = juce::jmax(50, bounds.getWidth() - labelWidth - 20); // Ensure minimum width
    
    // Validate component existence before setting bounds
    if (transformationSelector && bounds.getHeight() > 35) {
        transformationSelector->setBounds(labelWidth, 10, controlWidth, controlHeight);
    }
    if (rhythmPatternSelector && bounds.getHeight() > 65) {
        rhythmPatternSelector->setBounds(labelWidth, 40, controlWidth, controlHeight);
    }
    if (articulationStyleSelector && bounds.getHeight() > 95) {
        articulationStyleSelector->setBounds(labelWidth, 70, controlWidth, controlHeight);
    }
    if (humanizeSlider && bounds.getHeight() > 125) {
        humanizeSlider->setBounds(labelWidth, 100, controlWidth, controlHeight);
    }
    
    // Position buttons with validation
    if (applyButton && clearButton && bounds.getHeight() > 155) {
        int buttonWidth = (controlWidth - 10) / 2;
        applyButton->setBounds(labelWidth, 130, buttonWidth, controlHeight);
        clearButton->setBounds(labelWidth + buttonWidth + 10, 130, buttonWidth, controlHeight);
    }
}

void PatternControlsComponent::setCurrentPattern(const Pattern& pattern)
{
    try {
        if (!pattern.validate()) {
            juce::Logger::writeToLog("Warning: Invalid pattern provided to PatternControlsComponent. Details:");
            juce::Logger::writeToLog("  - Pattern length: " + juce::String(pattern.length));
            juce::Logger::writeToLog("  - Note count: " + juce::String(pattern.getNoteCount()));
            juce::Logger::writeToLog("  - Tempo: " + juce::String(pattern.tempo));
            return;
        }
        currentPattern = pattern;
        juce::Logger::writeToLog("Successfully set new pattern with " + 
                               juce::String(pattern.getNoteCount()) + " notes");
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog(createErrorMessage("setCurrentPattern", "Failed to set pattern", e));
    }
}

Pattern PatternControlsComponent::getCurrentPattern() const
{
    return currentPattern;
}

void PatternControlsComponent::initializeTransformationSelector()
{
    if (!transformationSelector) {
        juce::Logger::writeToLog("Error: Null transformationSelector in initializeTransformationSelector");
        return;
    }

    try {
        safeInitComboBox(transformationSelector.get(), "transformationSelector");
        
        // Add items with error checking
        struct TransformItem {
            const char* name;
            TransformationType type;
        };
        
        TransformItem items[] = {
            {"Step Up", TransformationType::StepUp},
            {"Step Down", TransformationType::StepDown},
            {"Up Two Down One", TransformationType::UpTwoDownOne},
            {"Skip One", TransformationType::SkipOne},
            {"Arch", TransformationType::Arch},
            {"Pendulum", TransformationType::Pendulum},
            {"Power Chord", TransformationType::PowerChord},
            {"Random Free", TransformationType::RandomFree},
            {"Random In Key", TransformationType::RandomInKey},
            {"Random Rhythmic", TransformationType::RandomRhythmic},
            {"Invert", TransformationType::Invert},
            {"Mirror", TransformationType::Mirror},
            {"Retrograde", TransformationType::Retrograde}
        };
        
        for (const auto& item : items) {
            try {
                transformationSelector->addItem(item.name, static_cast<int>(item.type) + 1);
            }
            catch (const std::exception& e) {
                juce::Logger::writeToLog("Failed to add item '" + juce::String(item.name) + 
                                       "' to transformation selector: " + e.what());
            }
        }
        
        transformationSelector->onChange = [this] { 
            try {
                handleTransformationChange(); 
            }
            catch (const std::exception& e) {
                juce::Logger::writeToLog(createErrorMessage("transformationSelector::onChange", 
                                       "Change handler failed", e));
            }
        };
        
        transformationSelector->setSelectedItemIndex(0, juce::dontSendNotification);
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog(createErrorMessage("initializeTransformationSelector", 
                               "Failed to initialize selector", e));
        transformationSelector->clear();
        transformationSelector->setEnabled(false);
        throw; // Re-throw to notify caller
    }
}

void PatternControlsComponent::initializeRhythmPatternSelector()
{
    if (!rhythmPatternSelector) return;

    try {
        rhythmPatternSelector->clear();
        rhythmPatternSelector->addItem("Regular", static_cast<int>(RhythmPattern::Regular) + 1);
        rhythmPatternSelector->addItem("Dotted Eighth", static_cast<int>(RhythmPattern::DottedEighth) + 1);
        rhythmPatternSelector->addItem("Swing", static_cast<int>(RhythmPattern::Swing) + 1);
        rhythmPatternSelector->addItem("Syncopated", static_cast<int>(RhythmPattern::Syncopated) + 1);
        rhythmPatternSelector->addItem("Random", static_cast<int>(RhythmPattern::Random) + 1);
        rhythmPatternSelector->addItem("Clave", static_cast<int>(RhythmPattern::Clave) + 1);
        
        rhythmPatternSelector->onChange = [this] { handleRhythmPatternChange(); };
        rhythmPatternSelector->setSelectedItemIndex(0, juce::dontSendNotification);
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error initializing rhythm pattern selector: " + juce::String(e.what()));
        rhythmPatternSelector->clear();
    }
}

void PatternControlsComponent::initializeArticulationStyleSelector()
{
    if (!articulationStyleSelector) return;

    try {
        articulationStyleSelector->clear();
        articulationStyleSelector->addItem("Legato", static_cast<int>(ArticulationStyle::Legato) + 1);
        articulationStyleSelector->addItem("Staccato", static_cast<int>(ArticulationStyle::Staccato) + 1);
        articulationStyleSelector->addItem("Mixed", static_cast<int>(ArticulationStyle::Mixed) + 1);
        articulationStyleSelector->addItem("Pattern", static_cast<int>(ArticulationStyle::Pattern) + 1);
        articulationStyleSelector->addItem("Random", static_cast<int>(ArticulationStyle::Random) + 1);
        
        articulationStyleSelector->onChange = [this] { handleArticulationStyleChange(); };
        articulationStyleSelector->setSelectedItemIndex(0, juce::dontSendNotification);
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error initializing articulation style selector: " + juce::String(e.what()));
        articulationStyleSelector->clear();
    }
}

void PatternControlsComponent::initializeHumanizeControls()
{
    if (!humanizeSlider) return;

    try {
        humanizeSlider->setRange(0.0, 1.0, 0.01);
        humanizeSlider->setValue(0.0, juce::dontSendNotification);
        humanizeSlider->setTextValueSuffix(" %");
        humanizeSlider->onValueChange = [this] { handleHumanizeChange(); };
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error initializing humanize controls: " + juce::String(e.what()));
        humanizeSlider->setValue(0.0, juce::dontSendNotification);
    }
}

void PatternControlsComponent::initializeButtons()
{
    if (!applyButton || !clearButton) return;

    try {
        applyButton->onClick = [this] { handleApplyButton(); };
        clearButton->onClick = [this] { handleClearButton(); };
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error initializing buttons: " + juce::String(e.what()));
    }
}

void PatternControlsComponent::handleTransformationChange()
{
    if (!transformationSelector) {
        juce::Logger::writeToLog("Error: Null transformationSelector in handleTransformationChange");
        return;
    }

    try {
        int index = transformationSelector->getSelectedItemIndex();
        if (index < 0) {
            juce::Logger::writeToLog("Warning: Invalid selection index in transformation selector");
            return;
        }

        if (onTransformationSelected) {
            auto type = static_cast<TransformationType>(index);
            juce::Logger::writeToLog("Transformation selected: " + transformationSelector->getText());
            onTransformationSelected(type);
        }
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog(createErrorMessage("handleTransformationChange", 
                               "Failed to handle transformation change", e));
        // Try to recover by resetting selection
        try {
            transformationSelector->setSelectedItemIndex(0, juce::dontSendNotification);
        }
        catch (...) {
            // If recovery fails, disable the control
            transformationSelector->setEnabled(false);
            juce::Logger::writeToLog("Disabled transformation selector due to unrecoverable error");
        }
    }
}

void PatternControlsComponent::handleRhythmPatternChange()
{
    if (!rhythmPatternSelector) return;

    try {
        if (onRhythmPatternSelected && rhythmPatternSelector->getSelectedItemIndex() >= 0) {
            auto pattern = static_cast<RhythmPattern>(rhythmPatternSelector->getSelectedItemIndex());
            onRhythmPatternSelected(pattern);
        }
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error handling rhythm pattern change: " + juce::String(e.what()));
    }
}

void PatternControlsComponent::handleArticulationStyleChange()
{
    if (!articulationStyleSelector) return;

    try {
        if (onArticulationStyleSelected && articulationStyleSelector->getSelectedItemIndex() >= 0) {
            auto style = static_cast<ArticulationStyle>(articulationStyleSelector->getSelectedItemIndex());
            onArticulationStyleSelected(style);
        }
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error handling articulation style change: " + juce::String(e.what()));
    }
}

void PatternControlsComponent::handleHumanizeChange()
{
    if (!humanizeSlider) return;

    try {
        double value = humanizeSlider->getValue();
        // Store the humanize value for later use
        currentHumanizeValue = juce::jlimit(0.0, 1.0, value);
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error handling humanize change: " + juce::String(e.what()));
        currentHumanizeValue = 0.0;
    }
}

void PatternControlsComponent::handleApplyButton()
{
    try {
        if (!currentPattern.validate()) {
            juce::Logger::writeToLog("Warning: Cannot apply invalid pattern. Validation failed.");
            // Notify user through UI
            if (applyButton) {
                applyButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
                juce::Timer::callAfterDelay(1000, [this]() {
                    if (applyButton) {
                        applyButton->setColour(juce::TextButton::buttonColourId, 
                                             getLookAndFeel().findColour(juce::TextButton::buttonColourId));
                    }
                });
            }
            return;
        }

        if (onPatternSelected) {
            juce::Logger::writeToLog("Applying pattern with " + 
                                   juce::String(currentPattern.getNoteCount()) + " notes");
            onPatternSelected(currentPattern);
        }
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog(createErrorMessage("handleApplyButton", "Failed to apply pattern", e));
        // Visual feedback for error
        if (applyButton) {
            applyButton->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            juce::Timer::callAfterDelay(1000, [this]() {
                if (applyButton) {
                    applyButton->setColour(juce::TextButton::buttonColourId, 
                                         getLookAndFeel().findColour(juce::TextButton::buttonColourId));
                }
            });
        }
    }
}

void PatternControlsComponent::handleClearButton()
{
    try {
        currentPattern = Pattern();
        if (onPatternSelected) {
            onPatternSelected(currentPattern);
        }
        
        // Reset controls to default state
        if (transformationSelector) transformationSelector->setSelectedItemIndex(0, juce::dontSendNotification);
        if (rhythmPatternSelector) rhythmPatternSelector->setSelectedItemIndex(0, juce::dontSendNotification);
        if (articulationStyleSelector) articulationStyleSelector->setSelectedItemIndex(0, juce::dontSendNotification);
        if (humanizeSlider) humanizeSlider->setValue(0.0, juce::dontSendNotification);
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Error handling clear button: " + juce::String(e.what()));
    }
} 