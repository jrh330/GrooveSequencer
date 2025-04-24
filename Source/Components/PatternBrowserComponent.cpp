void PatternBrowserComponent::loadPatternFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;
        
    auto json = JSON::parse(file);
    if (!json.isObject())
        return;
        
    auto* obj = json.getDynamicObject();
    if (!obj)
        return;
        
    auto entry = new PatternEntry();
    
    // Load metadata
    entry->name = obj->getProperty("name").toString();
    entry->type = "User";
    entry->style = obj->getProperty("style").toString();
    entry->modified = juce::Time(obj->getProperty("modified", static_cast<int64>(0)));
    
    // Load pattern data
    if (auto patternObj = obj->getProperty("pattern"))
    {
        entry->pattern.fromVar(patternObj);
    }
    
    patterns.add(entry);
    patternList->updateContent();
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
    auto file = patternsDirectory.getChildFile(name + ".pattern");
    
    // Create JSON object
    juce::DynamicObject* obj = new juce::DynamicObject();
    obj->setProperty("name", name);
    obj->setProperty("style", styleFilter->getText());
    obj->setProperty("modified", juce::Time::getCurrentTime().toMilliseconds());
    obj->setProperty("pattern", pattern.toVar());
    
    // Write to file
    juce::var json(obj);
    file.replaceWithText(JSON::toString(json));
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

void PatternBrowserComponent::paintRowBackground(juce::Graphics& g, int rowNumber,
                                               int width, int height, bool rowIsSelected)
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

// Update constructor to initialize filtered list
PatternBrowserComponent::PatternBrowserComponent()
{
    // ... existing initialization code ...
    
    // Initialize filters
    currentSearchText = "";
    currentStyleFilter = "All Styles";
    
    // Add mouse listener
    patternList->addMouseListener(this, false);
    
    // Initialize filtered indices with all patterns
    updateFilteredList();
}

// Update pattern management methods to refresh filtered list
void PatternBrowserComponent::addPattern(const Pattern& pattern, const juce::String& name)
{
    auto entry = new PatternEntry();
    entry->pattern = pattern;
    entry->name = name;
    entry->type = "User";
    entry->style = "Custom";
    entry->modified = juce::Time::getCurrentTime();
    
    patterns.add(entry);
    updateFilteredList();
}

void PatternBrowserComponent::deleteSelectedPattern()
{
    auto selectedRow = patternList->getSelectedRow();
    if (selectedRow >= 0 && selectedRow < filteredIndices.size())
    {
        int actualIndex = filteredIndices[selectedRow];
        if (patterns[actualIndex]->type == "User")
        {
            // Delete file if it exists
            auto file = patternsDirectory.getChildFile(patterns[actualIndex]->name + ".pattern");
            if (file.existsAsFile())
                file.deleteFile();
            
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