#ifndef GROOVE_SEQUENCER_PATTERN_H
#define GROOVE_SEQUENCER_PATTERN_H

#include <JuceHeader.h>
#include <vector>
#include <stdexcept>

// Constants for validation
namespace PatternConstants {
    constexpr int MIN_MIDI_NOTE = 0;
    constexpr int MAX_MIDI_NOTE = 127;
    constexpr float MIN_VELOCITY = 0.0f;
    constexpr float MAX_VELOCITY = 127.0f;
    constexpr float MIN_TIME = 0.0f;
    constexpr float MIN_DURATION = 0.001f;
    constexpr int MIN_ACCENT = 0;
    constexpr int MAX_ACCENT = 2;
    constexpr int MIN_LENGTH = 1;
    constexpr int MAX_LENGTH = 128;
    constexpr double MIN_TEMPO = 20.0;
    constexpr double MAX_TEMPO = 300.0;
    constexpr double MIN_GRID_SIZE = 0.0625; // 1/64 note
    constexpr double MAX_GRID_SIZE = 4.0;    // whole note
}

class InvalidNoteException : public std::runtime_error {
public:
    explicit InvalidNoteException(const std::string& message) 
        : std::runtime_error(message) {}
};

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
    {
        validate();
    }
    
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
    
    void validate() const {
        using namespace PatternConstants;
        
        if (pitch < MIN_MIDI_NOTE || pitch > MAX_MIDI_NOTE) {
            throw InvalidNoteException("Invalid MIDI note number: " + std::to_string(pitch));
        }
        if (velocity < MIN_VELOCITY || velocity > MAX_VELOCITY) {
            throw InvalidNoteException("Invalid velocity: " + std::to_string(velocity));
        }
        if (startTime < MIN_TIME) {
            throw InvalidNoteException("Invalid start time: " + std::to_string(startTime));
        }
        if (duration < MIN_DURATION) {
            throw InvalidNoteException("Invalid duration: " + std::to_string(duration));
        }
        if (accent < MIN_ACCENT || accent > MAX_ACCENT) {
            throw InvalidNoteException("Invalid accent: " + std::to_string(accent));
        }
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        try {
            validate();
            return true;
        } catch (const InvalidNoteException&) {
            return false;
        }
    }
    
    [[nodiscard]] juce::var toVar() const {
        validate(); // Ensure note is valid before serializing
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
            note.pitch = static_cast<int>(obj->getProperty("pitch"));
            note.velocity = static_cast<float>(static_cast<double>(obj->getProperty("velocity")));
            note.startTime = static_cast<float>(static_cast<double>(obj->getProperty("startTime")));
            note.duration = static_cast<float>(static_cast<double>(obj->getProperty("duration")));
            note.accent = static_cast<int>(obj->getProperty("accent"));
            note.active = static_cast<bool>(obj->getProperty("active"));
            note.isStaccato = static_cast<bool>(obj->getProperty("isStaccato"));
            note.isRest = static_cast<bool>(obj->getProperty("isRest"));
            note.validate();
        }
        return note;
    }
};

class Pattern {
public:
    Pattern(int len = 16, double tmp = 120.0, double grid = 0.25)
        : length_(validateLength(len))
        , tempo_(validateTempo(tmp))
        , gridSize_(validateGridSize(grid))
    {
        notes_.reserve(static_cast<size_t>(length_));
    }
    
    // Getters
    [[nodiscard]] int getLength() const noexcept { return length_; }
    [[nodiscard]] double getTempo() const noexcept { return tempo_; }
    [[nodiscard]] double getGridSize() const noexcept { return gridSize_; }
    [[nodiscard]] const std::vector<Note>& getNotes() const noexcept { return notes_; }
    std::vector<Note>& getNotes() noexcept { return notes_; }  // Non-const access for modifications
    
    // Setters with validation
    void setLength(int len) {
        length_ = validateLength(len);
        notes_.reserve(static_cast<size_t>(length_));
    }
    
    void setTempo(double tmp) {
        tempo_ = validateTempo(tmp);
    }
    
    void setGridSize(double grid) {
        gridSize_ = validateGridSize(grid);
    }
    
    [[nodiscard]] juce::var toVar() const {
        auto obj = std::make_unique<juce::DynamicObject>();
        obj->setProperty("length", length_);
        obj->setProperty("tempo", tempo_);
        obj->setProperty("gridSize", gridSize_);
        
        juce::Array<juce::var> notesArray;
        for (const auto& note : notes_) {
            notesArray.add(note.toVar());
        }
        obj->setProperty("notes", notesArray);
        
        return juce::var(obj.release());
    }
    
    static Pattern fromVar(const juce::var& v) {
        Pattern pattern;
        if (auto* obj = v.getDynamicObject()) {
            pattern.setLength(static_cast<int>(obj->getProperty("length")));
            pattern.setTempo(static_cast<double>(obj->getProperty("tempo")));
            pattern.setGridSize(static_cast<double>(obj->getProperty("gridSize")));
            
            if (auto* notesArray = obj->getProperty("notes").getArray()) {
                pattern.notes_.reserve(static_cast<size_t>(notesArray->size()));
                for (const auto& noteVar : *notesArray) {
                    pattern.notes_.push_back(Note::fromVar(noteVar));
                }
            }
        }
        return pattern;
    }
    
    void removeNote(size_t index) {
        if (index < notes_.size()) {
            notes_.erase(notes_.begin() + static_cast<std::ptrdiff_t>(index));
        }
    }
    
    void addNote(const Note& note) {
        note.validate();
        notes_.push_back(note);
    }
    
    void clear() {
        notes_.clear();
    }
    
    [[nodiscard]] bool isEmpty() const noexcept {
        return notes_.empty();
    }
    
    [[nodiscard]] size_t size() const noexcept {
        return notes_.size();
    }

    [[nodiscard]] size_t getNoteCount() const noexcept {
        return notes_.size();
    }

    [[nodiscard]] bool validate() const noexcept {
        try {
            validateLength(length_);
            validateTempo(tempo_);
            validateGridSize(gridSize_);
            
            for (const auto& note : notes_) {
                note.validate();
                if (note.startTime >= static_cast<float>(length_) || 
                    note.startTime + note.duration > static_cast<float>(length_)) {
                    return false;
                }
            }
            return true;
        } catch (...) {
            return false;
        }
    }

private:
    std::vector<Note> notes_;
    int length_{16};           // Pattern length in beats
    double tempo_{120.0};      // Tempo in BPM
    double gridSize_{0.25};    // Grid size in beats (0.25 = 16th notes)

    static int validateLength(int len) {
        using namespace PatternConstants;
        if (len < MIN_LENGTH || len > MAX_LENGTH) {
            throw std::invalid_argument("Invalid pattern length: " + std::to_string(len));
        }
        return len;
    }
    
    static double validateTempo(double tmp) {
        using namespace PatternConstants;
        if (tmp < MIN_TEMPO || tmp > MAX_TEMPO) {
            throw std::invalid_argument("Invalid tempo: " + std::to_string(tmp));
        }
        return tmp;
    }
    
    static double validateGridSize(double grid) {
        using namespace PatternConstants;
        if (grid < MIN_GRID_SIZE || grid > MAX_GRID_SIZE) {
            throw std::invalid_argument("Invalid grid size: " + std::to_string(grid));
        }
        return grid;
    }
};

#endif // GROOVE_SEQUENCER_PATTERN_H