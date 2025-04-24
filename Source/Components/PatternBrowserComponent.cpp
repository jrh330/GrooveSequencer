#include "PatternBrowserComponent.h"
#include <juce_data_structures/juce_data_structures.h>

namespace {
    constexpr const char* kPatternFileExtension = ".pattern";
    constexpr const char* kDefaultPatternsDir = "GrooveSequencer/Patterns";
}

enum class TableColumns
{
    Name = 1,
    Type,
    Style,
    Modified
};

PatternBrowserComponent::PatternBrowserComponent()
{
    // Initialize UI components
    patternList = std::make_unique<juce::TableListBox>();
    loadButton = std::make_unique<juce::TextButton>("Load");
    saveButton = std::make_unique<juce::TextButton>("Save");
    deleteButton = std::make_unique<juce::TextButton>("Delete");
    searchBox = std::make_unique<juce::TextEditor>();
    styleFilter = std::make_unique<juce::ComboBox>();
    
    // Add components
    addAndMakeVisible(patternList.get());
    addAndMakeVisible(loadButton.get());
    addAndMakeVisible(saveButton.get());
    addAndMakeVisible(deleteButton.get());
    addAndMakeVisible(searchBox.get());
    addAndMakeVisible(styleFilter.get());
    
    // Initialize table
    initializeTable();
    initializeButtons();
    initializeFilters();
    
    // Set up patterns directory
    patternsDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                           .getChildFile(kDefaultPatternsDir);
    
    juce::Result result = patternsDirectory.createDirectory();
    if (result.failed()) {
        // Log error but continue - we can still work with preset patterns
        juce::Logger::writeToLog("Failed to create patterns directory: " + result.getErrorMessage());
    }
    
    // Load patterns
    loadPresetPatterns();
    
    // Initialize filters
    currentSearchText = "";
    currentStyleFilter = "All Styles";
    
    // Add mouse listener
    patternList->addMouseListener(this, false);
    
    // Initialize filtered indices with all patterns
    updateFilteredList();
}

PatternBrowserComponent::~PatternBrowserComponent()
{
    patternList->removeMouseListener(this);
}

void PatternBrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PatternBrowserComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    
    // Layout filters at top
    auto topArea = bounds.removeFromTop(30);
    searchBox->setBounds(topArea.removeFromLeft(200));
    topArea.removeFromLeft(10);
    styleFilter->setBounds(topArea.removeFromLeft(150));
    
    // Layout buttons at bottom
    auto buttonArea = bounds.removeFromBottom(30);
    loadButton->setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    saveButton->setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    deleteButton->setBounds(buttonArea.removeFromLeft(100));
    
    // Pattern list takes remaining space
    bounds.removeFromTop(10);
    patternList->setBounds(bounds);
}

void PatternBrowserComponent::initializeTable()
{
    patternList->setModel(this);
    patternList->getHeader().addColumn("Name", static_cast<int>(TableColumns::Name), 200);
    patternList->getHeader().addColumn("Type", static_cast<int>(TableColumns::Type), 100);
    patternList->getHeader().addColumn("Style", static_cast<int>(TableColumns::Style), 100);
    patternList->getHeader().addColumn("Modified", static_cast<int>(TableColumns::Modified), 150);
    patternList->setMultipleSelectionEnabled(false);
}

void PatternBrowserComponent::initializeButtons()
{
    loadButton->onClick = [this]() {
        auto selectedRow = patternList->getSelectedRow();
        if (selectedRow >= 0 && selectedRow < filteredIndices.size())
        {
            int actualIndex = filteredIndices[selectedRow];
            if (onPatternSelected)
                onPatternSelected(patterns[actualIndex]->pattern);
        }
    };
    
    saveButton->onClick = []() {
        // TODO: Implement save dialog
    };
    
    deleteButton->onClick = [this]() {
        deleteSelectedPattern();
    };
}

void PatternBrowserComponent::initializeFilters()
{
    searchBox->setTextToShowWhenEmpty("Search patterns...", juce::Colours::grey);
    searchBox->onTextChange = [this]() {
        handleSearch(searchBox->getText());
    };
    
    styleFilter->addItem("All Styles", 1);
    styleFilter->addItem("Rock", 2);
    styleFilter->addItem("Jazz", 3);
    styleFilter->addItem("Latin", 4);
    styleFilter->addItem("Custom", 5);
    styleFilter->setSelectedId(1);
    
    styleFilter->onChange = [this]() {
        handleStyleFilter(styleFilter->getText());
    };
}

void PatternBrowserComponent::loadPresetPatterns()
{
    createBasicPatterns();
    createRockPatterns();
    createLatinPatterns();
    createJazzPatterns();
    
    // Load user patterns from directory
    for (const auto& file : patternsDirectory.findChildFiles(juce::File::findFiles, false, "*.pattern"))
    {
        loadPatternFromFile(file);
    }
}

void PatternBrowserComponent::createBasicPatterns()
{
    // TODO: Implement basic patterns
}

void PatternBrowserComponent::createRockPatterns()
{
    // TODO: Implement rock patterns
}

void PatternBrowserComponent::createLatinPatterns()
{
    // TODO: Implement latin patterns
}

void PatternBrowserComponent::createJazzPatterns()
{
    // TODO: Implement jazz patterns
}

void PatternBrowserComponent::loadPatternFromFile(const juce::File& file)
{
    if (!file.existsAsFile()) {
        juce::Logger::writeToLog("Pattern file does not exist: " + file.getFullPathName());
        return;
    }
        
    try {
        auto jsonText = file.loadFileAsString();
        auto jsonVar = juce::JSON::fromString(jsonText);
        
        if (!jsonVar.isObject()) {
            juce::Logger::writeToLog("Invalid JSON in pattern file: " + file.getFullPathName());
            return;
        }
            
        PatternEntry entry = PatternEntry::fromVar(jsonVar);
        if (!entry.validate()) {
            juce::Logger::writeToLog("Invalid pattern data in file: " + file.getFullPathName());
            return;
        }

        patterns.add(new PatternEntry(std::move(entry)));
        updateList();
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Exception while loading pattern file: " + juce::String(e.what()));
    }
}

void PatternBrowserComponent::saveCurrentPattern(const Pattern& pattern, const juce::String& name)
{
    // Create pattern entry
    auto entry = new PatternEntry();
    entry->pattern = pattern;
    entry->name = name;
    entry->type = "User";
    entry->style = styleFilter->getText();
    entry->modified = juce::Time::getCurrentTime();
    
    // Save to file
    savePatternToFile(pattern, name);
    
    // Add to list
    patterns.add(entry);
    patternList->updateContent();
}

void PatternBrowserComponent::savePatternToFile(const Pattern& pattern, const juce::String& name)
{
    if (name.isEmpty()) {
        juce::Logger::writeToLog("Cannot save pattern with empty name");
        return;
    }

    if (!pattern.validate()) {
        juce::Logger::writeToLog("Cannot save invalid pattern: " + name);
        return;
    }

    auto file = patternsDirectory.getChildFile(name + kPatternFileExtension);
    
    try {
        PatternEntry entry;
        entry.pattern = pattern;
        entry.name = name;
        entry.type = "User";
        entry.style = styleFilter->getText();
        entry.modified = juce::Time::getCurrentTime();

        if (!entry.validate()) {
            juce::Logger::writeToLog("Cannot save invalid pattern entry: " + name);
            return;
        }

        // Write to file
        if (!file.replaceWithText(juce::JSON::toString(entry.toVar()))) {
            juce::Logger::writeToLog("Failed to write pattern file: " + file.getFullPathName());
        }
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Exception while saving pattern file: " + juce::String(e.what()));
    }
}

void PatternBrowserComponent::updateFilteredList()
{
    filteredIndices.clearQuick();
    
    for (int i = 0; i < patterns.size(); ++i)
    {
        if (patternMatchesFilter(patterns[i]))
        {
            filteredIndices.add(i);
        }
    }
    
    patternList->updateContent();
}

bool PatternBrowserComponent::patternMatchesFilter(const PatternEntry* entry) const
{
    if (!currentSearchText.isEmpty())
    {
        if (!entry->name.containsIgnoreCase(currentSearchText) &&
            !entry->style.containsIgnoreCase(currentSearchText) &&
            !entry->type.containsIgnoreCase(currentSearchText))
        {
            return false;
        }
    }
    
    if (!currentStyleFilter.isEmpty() && currentStyleFilter != "All Styles")
    {
        if (entry->style != currentStyleFilter)
        {
            return false;
        }
    }
    
    return true;
}

// Update the handleSearch method
void PatternBrowserComponent::handleSearch(const juce::String& searchText)
{
    currentSearchText = searchText;
    updateFilteredList();
}

// Update the handleStyleFilter method
void PatternBrowserComponent::handleStyleFilter(const juce::String& style)
{
    currentStyleFilter = style;
    updateFilteredList();
}

// Update TableListBoxModel methods to use filtered indices
int PatternBrowserComponent::getNumRows()
{
    return filteredIndices.size();
}

void PatternBrowserComponent::paintRowBackground(
    juce::Graphics& g,
    int rowNumber,
    int /*width*/,
    int /*height*/,
    bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue.withAlpha(0.2f));
    else if (rowNumber % 2)
        g.fillAll(juce::Colours::grey.withAlpha(0.05f));
}

void PatternBrowserComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId,
                                      int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= filteredIndices.size())
        return;
        
    int actualIndex = filteredIndices[rowNumber];
    if (actualIndex >= patterns.size())
        return;
        
    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colours::lightgrey);
    g.setFont(14.0f);
    
    auto entry = patterns[actualIndex];
    juce::String text;
    
    switch (static_cast<TableColumns>(columnId))
    {
        case TableColumns::Name:
            text = entry->name;
            break;
        case TableColumns::Type:
            text = entry->type;
            break;
        case TableColumns::Style:
            text = entry->style;
            break;
        case TableColumns::Modified:
            text = entry->modified.toString(true, true, true, true);
            break;
    }
    
    g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft);
}

// Update pattern management methods to refresh filtered list
void PatternBrowserComponent::addPattern(const Pattern& pattern, const juce::String& name)
{
    if (name.isEmpty()) {
        juce::Logger::writeToLog("Cannot add pattern with empty name");
        return;
    }

    if (!pattern.validate()) {
        juce::Logger::writeToLog("Cannot add invalid pattern: " + name);
        return;
    }

    auto entry = std::make_unique<PatternEntry>();
    entry->pattern = pattern;
    entry->name = name;
    entry->type = "User";
    entry->style = "Custom";
    entry->modified = juce::Time::getCurrentTime();

    if (!entry->validate()) {
        juce::Logger::writeToLog("Cannot add invalid pattern entry: " + name);
        return;
    }

    patterns.add(entry.release());
    updateFilteredList();
}

void PatternBrowserComponent::deleteSelectedPattern()
{
    auto selectedRow = patternList->getSelectedRow();
    if (selectedRow >= 0 && selectedRow < filteredIndices.size()) {
        int actualIndex = filteredIndices[selectedRow];
        if (patterns[actualIndex]->type == "User") {
            // Delete file if it exists
            auto file = patternsDirectory.getChildFile(patterns[actualIndex]->name + kPatternFileExtension);
            if (file.existsAsFile()) {
                juce::Result result = file.deleteFile() ? juce::Result::ok() : juce::Result::fail("Failed to delete file");
                if (result.failed()) {
                    juce::Logger::writeToLog("Failed to delete pattern file: " + result.getErrorMessage());
                    return;
                }
            }
            
            patterns.remove(actualIndex);
            updateFilteredList();
        }
    }
}

// Add mouse handling for double-click
void PatternBrowserComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    int row = patternList->getRowContainingPosition(event.x, event.y);
    if (row >= 0 && row < patterns.size())
    {
        if (onPatternDoubleClicked)
            onPatternDoubleClicked(patterns[row]->pattern);
    }
}

void PatternBrowserComponent::loadPattern(const juce::String& name)
{
    auto file = patternsDirectory.getChildFile(name + kPatternFileExtension);
    if (file.existsAsFile())
    {
        try {
            auto jsonText = file.loadFileAsString();
            auto jsonVar = juce::JSON::fromString(jsonText);
            
            if (!jsonVar.isObject()) {
                juce::Logger::writeToLog("Invalid JSON in pattern file: " + file.getFullPathName());
                return;
            }
            
            PatternEntry entry = PatternEntry::fromVar(jsonVar);
            if (!entry.validate()) {
                juce::Logger::writeToLog("Invalid pattern entry");
                return;
            }
            
            patterns.add(new PatternEntry(std::move(entry)));
            updateList();
        }
        catch (const std::exception& e) {
            juce::Logger::writeToLog("Exception while loading pattern: " + juce::String(e.what()));
        }
    }
}

void PatternBrowserComponent::updateList()
{
    patternList->updateContent();
    patternList->repaint();
} 