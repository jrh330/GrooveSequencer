#ifndef GROOVE_SEQUENCER_PATTERN_H
#define GROOVE_SEQUENCER_PATTERN_H

#include <JuceHeader.h>
#include <vector>

struct Note {
    int pitch{60};            // MIDI note number (0-127)
    float velocity{100.0f};   // Note velocity (0-127)
    float startTime{0.0f};    // Start time in beats
    float duration{1.0f};     // Duration in beats
    int accent{0};            // 0 = no accent, 1 = medium, 2 = strong
    bool active{true};        // Whether this note is active
    bool isStaccato{false};   // Whether this note is staccato
    bool isRest{false};       // Whether this note is a rest
    
    Note() = default;
    
    Note(int p, float v, float s, float d, int a = 0, bool act = true)
        : pitch(p)
        , velocity(v)
        , startTime(s)
        , duration(d)
        , accent(a)
        , active(act)
    {}
    
    bool operator==(const Note& other) const {
        return pitch == other.pitch &&
               std::abs(velocity - other.velocity) < 0.001f &&
               std::abs(startTime - other.startTime) < 0.001f &&
               std::abs(duration - other.duration) < 0.001f &&
               accent == other.accent &&
               active == other.active &&
               isStaccato == other.isStaccato &&
               isRest == other.isRest;
    }
    
    bool operator!=(const Note& other) const {
        return !(*this == other);
    }
    
    [[nodiscard]] bool isValid() const {
        return pitch >= 0 && pitch < 128 &&
               velocity >= 0.0f && velocity <= 127.0f &&
               startTime >= 0.0f &&
               duration > 0.0f &&
               accent >= 0 && accent <= 2;
    }
    
    [[nodiscard]] juce::var toVar() const {
        auto obj = std::make_unique<juce::DynamicObject>();
        obj->setProperty("pitch", pitch);
        obj->setProperty("velocity", velocity);
        obj->setProperty("startTime", startTime);
        obj->setProperty("duration", duration);
        obj->setProperty("accent", accent);
        obj->setProperty("active", active);
        obj->setProperty("isStaccato", isStaccato);
        obj->setProperty("isRest", isRest);
        return juce::var(obj.release());
    }
    
    static Note fromVar(const juce::var& v) {
        Note note;
        if (auto* obj = v.getDynamicObject()) {
            note.pitch = obj->getProperty("pitch");
            note.velocity = obj->getProperty("velocity");
            note.startTime = obj->getProperty("startTime");
            note.duration = obj->getProperty("duration");
            note.accent = obj->getProperty("accent");
            note.active = obj->getProperty("active");
            note.isStaccato = obj->getProperty("isStaccato");
            note.isRest = obj->getProperty("isRest");
        }
        return note;
    }
};

struct Pattern {
    std::vector<Note> notes;
    int length{16};           // Pattern length in beats
    double tempo{120.0};      // Tempo in BPM
    double gridSize{0.25};    // Grid size in beats (0.25 = 16th notes)
    
    Pattern(int len = 16, double tmp = 120.0, double grid = 0.25)
        : length(len)
        , tempo(tmp)
        , gridSize(grid)
    {
        notes.reserve(static_cast<size_t>(length));
    }
    
    [[nodiscard]] juce::var toVar() const {
        auto obj = std::make_unique<juce::DynamicObject>();
        obj->setProperty("length", length);
        obj->setProperty("tempo", tempo);
        obj->setProperty("gridSize", gridSize);
        
        juce::Array<juce::var> notesArray;
        for (const auto& note : notes) {
            notesArray.add(note.toVar());
        }
        obj->setProperty("notes", notesArray);
        
        return juce::var(obj.release());
    }
    
    static Pattern fromVar(const juce::var& v) {
        Pattern pattern;
        if (auto* obj = v.getDynamicObject()) {
            pattern.length = obj->getProperty("length");
            pattern.tempo = obj->getProperty("tempo");
            pattern.gridSize = obj->getProperty("gridSize");
            
            if (auto* notesArray = obj->getProperty("notes").getArray()) {
                pattern.notes.reserve(static_cast<size_t>(notesArray->size()));
                for (const auto& noteVar : *notesArray) {
                    pattern.notes.push_back(Note::fromVar(noteVar));
                }
            }
        }
        return pattern;
    }
    
    void removeNote(size_t index) {
        if (index < notes.size()) {
            notes.erase(notes.begin() + static_cast<std::ptrdiff_t>(index));
        }
    }
    
    void addNote(const Note& note) {
        notes.push_back(note);
    }
    
    void clear() {
        notes.clear();
    }
    
    [[nodiscard]] bool isEmpty() const {
        return notes.empty();
    }
    
    [[nodiscard]] size_t size() const {
        return notes.size();
    }

    [[nodiscard]] size_t getNoteCount() const {
        return notes.size();
    }

    [[nodiscard]] bool validate() const {
        // Check pattern parameters
        if (length <= 0 || tempo <= 0.0 || gridSize <= 0.0) {
            return false;
        }

        // Check all notes
        for (const auto& note : notes) {
            if (!note.isValid()) {
                return false;
            }
            // Additional pattern-specific validation
            if (note.startTime >= static_cast<float>(length) || 
                note.startTime + note.duration > static_cast<float>(length)) {
                return false;
            }
        }
        return true;
    }
};

#endif // GROOVE_SEQUENCER_PATTERN_H