#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "../Models/PatternEntry.h"

/**
 * @brief A component that displays and manages a list of patterns
 */
class PatternBrowserComponent : public juce::Component,
                              public juce::TableListBoxModel
{
public:
    PatternBrowserComponent();
    ~PatternBrowserComponent() override;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    
    //==============================================================================
    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int /*rowNumber*/, int /*columnId*/, bool /*isRowSelected*/, 
                                          juce::Component* /*existingComponentToUpdate*/) override { return nullptr; }
    
    //==============================================================================
    // Pattern management
    /**
     * @brief Adds a new pattern to the browser
     * @param pattern The pattern to add
     * @param name The name of the pattern
     */
    void addPattern(const Pattern& pattern, const juce::String& name);
    
    /**
     * @brief Loads preset patterns into the browser
     */
    void loadPresetPatterns();
    
    /**
     * @brief Saves the current pattern with the given name
     * @param pattern The pattern to save
     * @param name The name to save it under
     */
    void saveCurrentPattern(const Pattern& pattern, const juce::String& name);
    
    /**
     * @brief Deletes the currently selected pattern
     */
    void deleteSelectedPattern();
    
    //==============================================================================
    // Callbacks
    /** Called when a pattern is selected */
    std::function<void(const Pattern&)> onPatternSelected;
    
    /** Called when a pattern is double-clicked */
    std::function<void(const Pattern&)> onPatternDoubleClicked;

    //==============================================================================
    // Pattern loading and management
    void loadPattern(const juce::String& name);
    void updateList();

private:
    //==============================================================================
    // UI Components
    std::unique_ptr<juce::TableListBox> patternList;
    std::unique_ptr<juce::TextButton> loadButton;
    std::unique_ptr<juce::TextButton> saveButton;
    std::unique_ptr<juce::TextButton> deleteButton;
    std::unique_ptr<juce::TextEditor> searchBox;
    std::unique_ptr<juce::ComboBox> styleFilter;
    
    //==============================================================================
    // Data
    juce::OwnedArray<PatternEntry> patterns;
    juce::Array<int> filteredIndices;  // Indices of patterns that match current filter
    juce::File patternsDirectory;
    juce::String currentSearchText;
    juce::String currentStyleFilter;
    
    //==============================================================================
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
    [[nodiscard]] bool patternMatchesFilter(const PatternEntry* entry) const;
    
    // Preset patterns
    void createBasicPatterns();
    void createRockPatterns();
    void createLatinPatterns();
    void createJazzPatterns();
    
    //==============================================================================
    // Pattern loading and management
    void loadPatterns();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternBrowserComponent)
}; 