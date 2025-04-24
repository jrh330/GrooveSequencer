#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class PatternBrowserComponent : public juce::Component,
                              public juce::TableListBoxModel,
                              public juce::MouseListener
{
public:
    PatternBrowserComponent();
    ~PatternBrowserComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Pattern management
    void addPattern(const Pattern& pattern, const juce::String& name);
    void loadPresetPatterns();
    void saveCurrentPattern(const Pattern& pattern, const juce::String& name);
    void deleteSelectedPattern();
    
    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, 
                                          juce::Component* existingComponentToUpdate) override;
    
    // Callbacks
    std::function<void(const Pattern&)> onPatternSelected;
    std::function<void(const Pattern&)> onPatternDoubleClicked;

private:
    struct PatternEntry {
        Pattern pattern;
        juce::String name;
        juce::String type;      // "Preset" or "User"
        juce::String style;     // e.g., "Rock", "Jazz", "Latin"
        juce::Time modified;
    };
    
    // UI Components
    std::unique_ptr<juce::TableListBox> patternList;
    std::unique_ptr<juce::TextButton> loadButton;
    std::unique_ptr<juce::TextButton> saveButton;
    std::unique_ptr<juce::TextButton> deleteButton;
    std::unique_ptr<juce::TextEditor> searchBox;
    std::unique_ptr<juce::ComboBox> styleFilter;
    
    // Data
    juce::OwnedArray<PatternEntry> patterns;
    juce::Array<int> filteredIndices;  // Indices of patterns that match current filter
    juce::File patternsDirectory;
    juce::String currentSearchText;
    juce::String currentStyleFilter;
    
    // Helper methods
    void initializeTable();
    void initializeButtons();
    void initializeFilters();
    void updatePatternList();
    void handlePatternSelection();
    void handleDoubleClick();
    void handleSearch(const juce::String& searchText);
    void handleStyleFilter(const juce::String& style);
    
    void loadPatternFromFile(const juce::File& file);
    void savePatternToFile(const Pattern& pattern, const juce::String& name);
    
    // Filtering methods
    void updateFilteredList();
    bool patternMatchesFilter(const PatternEntry* entry) const;
    
    // Mouse handling
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    
    // Preset patterns
    void createBasicPatterns();
    void createRockPatterns();
    void createLatinPatterns();
    void createJazzPatterns();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternBrowserComponent)
}; 