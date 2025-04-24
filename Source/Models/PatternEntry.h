#pragma once

#include <JuceHeader.h>
#include "../Pattern.h"

/**
 * @brief Represents a pattern entry in the browser with metadata
 */
struct PatternEntry {
    Pattern pattern;
    juce::String name;
    juce::String type;      // "Preset" or "User"
    juce::String style;     // e.g., "Rock", "Jazz", "Latin"
    juce::Time modified;

    PatternEntry() = default;
    
    PatternEntry(const Pattern& p, const juce::String& n, const juce::String& t, const juce::String& s)
        : pattern(p)
        , name(n)
        , type(t)
        , style(s)
        , modified(juce::Time::getCurrentTime())
    {}
    
    PatternEntry(const PatternEntry& other)
        : pattern(other.pattern)
        , name(other.name)
        , type(other.type)
        , style(other.style)
        , modified(other.modified)
    {}
    
    PatternEntry(PatternEntry&& other) noexcept
        : pattern(std::move(other.pattern))
        , name(std::move(other.name))
        , type(std::move(other.type))
        , style(std::move(other.style))
        , modified(other.modified)
    {}
    
    PatternEntry& operator=(PatternEntry other) noexcept {
        swap(*this, other);
        return *this;
    }
    
    friend void swap(PatternEntry& first, PatternEntry& second) noexcept {
        using std::swap;
        swap(first.pattern, second.pattern);
        swap(first.name, second.name);
        swap(first.type, second.type);
        swap(first.style, second.style);
        swap(first.modified, second.modified);
    }

    /**
     * @brief Validates the entry's data
     */
    [[nodiscard]] bool validate() const {
        return !name.isEmpty() && 
               !type.isEmpty() && 
               !style.isEmpty() && 
               pattern.validate() &&
               (type == "Preset" || type == "User");
    }

    /**
     * @brief Converts the entry to a var for serialization
     */
    [[nodiscard]] juce::var toVar() const {
        auto obj = std::make_unique<juce::DynamicObject>();
        obj->setProperty("name", name);
        obj->setProperty("type", type);
        obj->setProperty("style", style);
        obj->setProperty("modified", static_cast<double>(modified.toMilliseconds()));
        obj->setProperty("pattern", pattern.toVar());
        return juce::var(obj.release());
    }

    /**
     * @brief Creates a PatternEntry from a var
     * @return A pair containing the entry and a boolean indicating success
     */
    static PatternEntry fromVar(const juce::var& v) {
        PatternEntry entry;
        
        if (auto* obj = v.getDynamicObject()) {
            entry.name = obj->getProperty(juce::Identifier("name")).toString();
            entry.type = obj->getProperty(juce::Identifier("type")).toString();
            entry.style = obj->getProperty(juce::Identifier("style")).toString();
            
            auto modifiedVar = obj->getProperty(juce::Identifier("modified"));
            if (!modifiedVar.isVoid()) {
                double milliseconds = modifiedVar.toString().getDoubleValue();
                entry.modified = juce::Time(static_cast<int64_t>(milliseconds));
            } else {
                entry.modified = juce::Time::getCurrentTime();
            }
            
            if (auto patternVar = obj->getProperty(juce::Identifier("pattern"))) {
                entry.pattern = Pattern::fromVar(patternVar);
            }
        }
        
        return entry;
    }
    
    [[nodiscard]] bool isPreset() const { return type == "Preset"; }
    [[nodiscard]] bool isUser() const { return type == "User"; }
    [[nodiscard]] juce::String getDisplayName() const { return name + " (" + style + ")"; }
}; 