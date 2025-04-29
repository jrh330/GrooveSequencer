#include "PatternTransformer.h"
#include <JuceHeader.h>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>

namespace PTLogger {
    void log(LogLevel level, const std::string& message, const std::string& function) {
        std::string levelStr;
        switch (level) {
            case LogLevel::Debug: levelStr = "DEBUG"; break;
            case LogLevel::Info: levelStr = "INFO"; break;
            case LogLevel::Warning: levelStr = "WARN"; break;
            case LogLevel::Error: levelStr = "ERROR"; break;
        }
        
        std::stringstream ss;
        ss << "[" << levelStr << "] " << function << ": " << message;
        juce::Logger::writeToLog(ss.str());
    }

    std::string noteToString(const Note& note) {
        std::stringstream ss;
        ss << "Note{pitch=" << note.pitch
           << ", velocity=" << note.velocity
           << ", startTime=" << note.startTime
           << ", duration=" << note.duration
           << ", accent=" << note.accent
           << ", active=" << note.active
           << ", staccato=" << note.isStaccato
           << ", rest=" << note.isRest << "}";
        return ss.str();
    }

    std::string notesToString(const std::vector<Note>& notes) {
        std::stringstream ss;
        ss << "Notes[" << notes.size() << "]={";
        for (size_t i = 0; i < notes.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << noteToString(notes[i]);
        }
        ss << "}";
        return ss.str();
    }

    std::string rhythmPatternToString(RhythmPattern pattern) {
        switch (pattern) {
            case RhythmPattern::Regular: return "Regular";
            case RhythmPattern::Dotted: return "Dotted";
            case RhythmPattern::Swing: return "Swing";
            case RhythmPattern::Syncopated: return "Syncopated";
            case RhythmPattern::Random: return "Random";
            case RhythmPattern::Clave: return "Clave";
            case RhythmPattern::LongShort: return "LongShort";
            case RhythmPattern::ShortLong: return "ShortLong";
            case RhythmPattern::LongShortShort: return "LongShortShort";
            case RhythmPattern::ShortShortLong: return "ShortShortLong";
            case RhythmPattern::DottedEighth: return "DottedEighth";
            case RhythmPattern::Triplet: return "Triplet";
            case RhythmPattern::Straight: return "Straight";
            case RhythmPattern::ThreeTwoClave: return "ThreeTwoClave";
            case RhythmPattern::TwoThreeClave: return "TwoThreeClave";
            case RhythmPattern::Shuffle: return "Shuffle";
            case RhythmPattern::Custom: return "Custom";
        }
        return "Unknown";
    }

    std::string transformationTypeToString(TransformationType type) {
        switch (type) {
            case TransformationType::StepUp: return "StepUp";
            case TransformationType::StepDown: return "StepDown";
            case TransformationType::UpTwoDownOne: return "UpTwoDownOne";
            case TransformationType::SkipOne: return "SkipOne";
            case TransformationType::Arch: return "Arch";
            case TransformationType::Pendulum: return "Pendulum";
            case TransformationType::PowerChord: return "PowerChord";
            case TransformationType::RandomFree: return "RandomFree";
            case TransformationType::RandomInKey: return "RandomInKey";
            case TransformationType::RandomRhythmic: return "RandomRhythmic";
            case TransformationType::Invert: return "Invert";
            case TransformationType::Mirror: return "Mirror";
            case TransformationType::Retrograde: return "Retrograde";
            default: return "Unknown";
        }
    }

    std::string articulationStyleToString(ArticulationStyle style) {
        switch (style) {
            case ArticulationStyle::Legato: return "Legato";
            case ArticulationStyle::Staccato: return "Staccato";
            case ArticulationStyle::Mixed: return "Mixed";
            case ArticulationStyle::Accented: return "Accented";
            case ArticulationStyle::Random: return "Random";
            case ArticulationStyle::Pattern: return "Pattern";
            case ArticulationStyle::Normal: return "Normal";
            case ArticulationStyle::AlternatingStaccato: return "AlternatingStaccato";
            case ArticulationStyle::OffbeatAccent: return "OffbeatAccent";
            case ArticulationStyle::Custom: return "Custom";
        }
        return "Unknown";
    }
}

PatternTransformer::PatternTransformer()
    : currentRhythm(RhythmPattern::Regular)
    , currentArticulation(ArticulationStyle::Legato)
    , currentGridSize(0.25) // Default to 16th notes
    , transformationType(TransformationType::StepUp)
    , rng(std::time(nullptr))
{
    // Initialize default scale (C major)
    currentScale.root = 60; // Middle C
    currentScale.intervals = {0, 2, 4, 5, 7, 9, 11}; // Major scale intervals
    
    // Initialize default random parameters
    randomParams = RandomParameters{};
}

void PatternTransformer::setSeedNotes(const std::vector<Note>& seeds) {
    seedNotes = seeds;
}

Pattern PatternTransformer::generatePattern(TransformationType type, int length)
{
    Pattern result;
    result.setLength(length);
    result.getNotes() = generateNotes(length);
    return result;
}

Pattern PatternTransformer::transformPattern(const Pattern& source, TransformationType type)
{
    Pattern result;
    result.getNotes() = applyTransformation(source.getNotes(), type);
    return result;
}

std::vector<Note> PatternTransformer::previewTransformation(const Pattern& pattern, TransformationType type)
{
    Pattern preview = transformPattern(pattern, type);
    return preview.getNotes();
}

// Implementation of transformation helpers
std::vector<Note> PatternTransformer::applyStepUp(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    newNote.pitch += 1; // Move up one semitone
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyStepDown(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    newNote.pitch -= 1; // Move down one semitone
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyUpTwoDownOne(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Determine if we should go up or down based on the pattern
    if (input.size() >= 2) {
        Note secondLastNote = input[input.size() - 2];
        if (lastNote.pitch > secondLastNote.pitch) {
            // We just went up, so go down
            newNote.pitch -= 1;
        } else {
            // We just went down or stayed same, so go up
            newNote.pitch += 2;
        }
    } else {
        // First transformation, go up
        newNote.pitch += 2;
    }
    
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applySkipOne(const std::vector<Note>& input)
{
    std::vector<Note> result;
    result.reserve(input.size());
    
    for (size_t i = 0; i < input.size(); i++) {
        if (i % 2 == 0) {  // Only keep every other note
            result.push_back(input[i]);
        }
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyAlternateOctave(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    newNote.pitch += 12; // Move up an octave
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyInversion(const std::vector<Note>& input)
{
    if (input.empty()) return input;
    
    std::vector<Note> result = input;
    int rootPitch = input[0].pitch;
    
    for (size_t i = 1; i < result.size(); i++) {
        // Invert intervals around root note
        int interval = rootPitch - input[i].pitch;
        result[i].pitch = rootPitch + interval;
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyMirror(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    
    // Add reversed copy of input
    for (auto it = input.rbegin(); it != input.rend(); ++it) {
        Note mirroredNote = *it;
        mirroredNote.startTime = result.back().startTime + result.back().duration;
        result.push_back(mirroredNote);
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyRetrograde(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    std::reverse(result.begin(), result.end());
    
    // Adjust start times to maintain sequence
    double currentTime = 0.0;
    for (auto& note : result) {
        note.startTime = currentTime;
        currentTime += note.duration;
    }
    
    return result;
}

void PatternTransformer::setScale(const Scale& scale) {
    currentScale = scale;
}

// Utility functions
int PatternTransformer::getNextScaleNote(int currentPitch, int steps) {
    int octave = currentPitch / 12;
    int note = currentPitch % 12;
    
    // Find current scale degree
    auto it = std::find(currentScale.intervals.begin(), currentScale.intervals.end(), note);
    if (it == currentScale.intervals.end()) {
        // If not in scale, snap to nearest scale note
        return snapToScale(currentPitch);
    }
    
    int currentDegree = std::distance(currentScale.intervals.begin(), it);
    int newDegree = currentDegree + steps;
    
    // Handle octave changes
    int octaveShift = newDegree / currentScale.intervals.size();
    newDegree = newDegree % currentScale.intervals.size();
    if (newDegree < 0) {
        newDegree += currentScale.intervals.size();
        octaveShift--;
    }
    
    return (octave + octaveShift) * 12 + currentScale.intervals[newDegree];
}

int PatternTransformer::snapToScale(int pitch) {
    int octave = pitch / 12;
    int note = pitch % 12;
    
    // Find closest scale note
    int closest = currentScale.intervals[0];
    int minDist = 12;
    
    for (int interval : currentScale.intervals) {
        int dist = std::abs(note - interval);
        if (dist < minDist) {
            minDist = dist;
            closest = interval;
        }
    }
    
    return octave * 12 + closest;
}

Note PatternTransformer::createNote(int pitch, double startTime, double duration, int velocity) {
    Note note;
    note.pitch = pitch;
    note.startTime = startTime;
    note.duration = duration;
    note.velocity = velocity;
    return note;
}

// New pattern implementations
std::vector<Note> PatternTransformer::applyArch(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    const int midPoint = static_cast<int>(input.size()) / 2;
    
    for (int i = 0; i < midPoint; i++) {
        result[i].pitch = getNextScaleNote(result[i].pitch, 1);  // Step up
    }
    for (int i = midPoint; i < static_cast<int>(input.size()); i++) {
        result[i].pitch = getNextScaleNote(result[i].pitch, -1);  // Step down
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyPendulum(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    bool goingUp = true;
    
    for (size_t i = 1; i < result.size(); i++) {
        if (goingUp) {
            result[i].pitch = getNextScaleNote(result[i-1].pitch, 2);
        } else {
            result[i].pitch = getNextScaleNote(result[i-1].pitch, -2);
        }
        goingUp = !goingUp;
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyPowerChord(const std::vector<Note>& input)
{
    std::vector<Note> result;
    result.reserve(input.size() * 2);  // Each note becomes two notes
    
    for (const auto& note : input) {
        // Add root note
        result.push_back(note);
        
        // Add fifth above
        Note fifth = note;
        fifth.pitch = getNextScaleNote(note.pitch, 4);  // Perfect fifth
        result.push_back(fifth);
    }
    
    return result;
}

void PatternTransformer::setRandomParameters(const RandomParameters& params) {
    randomParams = params;
}

std::vector<Note> PatternTransformer::applyRandomFree(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    
    for (auto& note : result) {
        // Random pitch within an octave range
        int randomStep = getRandomInt(-6, 6);
        note.pitch = note.pitch + randomStep;
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyRandomInKey(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    
    for (auto& note : result) {
        // Random step within scale
        int randomStep = getRandomInt(-3, 3);
        note.pitch = getNextScaleNote(note.pitch, randomStep);
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyRandomRhythmic(const std::vector<Note>& input)
{
    std::vector<Note> result = input;
    
    for (auto& note : result) {
        // Randomly adjust duration and start time slightly
        double durationMod = getRandomDouble(0.5, 1.5);
        note.duration *= durationMod;
        
        // Ensure note doesn't overlap with next note
        if (note.duration > 1.0) note.duration = 1.0;
    }
    
    return result;
}

void PatternTransformer::setRhythmPattern(RhythmPattern pattern) {
    currentRhythm = pattern;
}

void PatternTransformer::setArticulationStyle(ArticulationStyle style) {
    currentArticulation = style;
}

void PatternTransformer::setGridSize(double size) {
    currentGridSize = size;
}

Pattern PatternTransformer::generatePatternWithRhythm(const std::vector<Note>& input, RhythmPattern pattern)
{
    Pattern result;
    result.getNotes() = applyRhythmPattern(input, pattern);
    return result;
}

double PatternTransformer::calculateNoteDuration(int position, RhythmPattern pattern) {
    switch (pattern) {
        case RhythmPattern::Regular:
        case RhythmPattern::Straight:
            return 1.0;
        case RhythmPattern::Dotted:
            return position % 2 == 0 ? 1.5 : 0.5;
        case RhythmPattern::Triplet:
            return 0.666667;
        case RhythmPattern::Swing:
            return position % 2 == 0 ? 0.666667 : 0.333333;
        case RhythmPattern::Syncopated:
            return position % 3 == 0 ? 1.5 : 0.75;
        case RhythmPattern::ThreeTwoClave:
            return isThreeTwoClave ? (position % 2 == 0 ? 1.5 : 0.5) : 1.0;
        case RhythmPattern::TwoThreeClave:
            return !isThreeTwoClave ? (position % 2 == 0 ? 1.5 : 0.5) : 1.0;
        case RhythmPattern::Shuffle:
            return position % 2 == 0 ? 0.75 : 0.25;
        case RhythmPattern::Custom:
            return 1.0;
        case RhythmPattern::Random:
            return getRandomDouble(0.5, 1.5);
        case RhythmPattern::Clave:
            return position % 2 == 0 ? 1.0 : 0.5;
        case RhythmPattern::LongShort:
            return position % 2 == 0 ? 1.5 : 0.5;
        case RhythmPattern::ShortLong:
            return position % 2 == 0 ? 0.5 : 1.5;
        case RhythmPattern::LongShortShort:
            return position % 3 == 0 ? 1.5 : 0.5;
        case RhythmPattern::ShortShortLong:
            return position % 3 == 2 ? 1.5 : 0.5;
        case RhythmPattern::DottedEighth:
            return position % 3 == 0 ? 1.5 : 0.75;
    }
    return 1.0;
}

bool PatternTransformer::shouldBeStaccato(int position, ArticulationStyle style) {
    switch (style) {
        case ArticulationStyle::Legato:
            return false;
        case ArticulationStyle::Staccato:
            return true;
        case ArticulationStyle::Mixed:
            return position % 2 == 0;
        case ArticulationStyle::Accented:
            return position % 2 == 0;
        case ArticulationStyle::Random:
            return getRandomDouble(0.0, 1.0) > 0.5;
        case ArticulationStyle::Pattern:
            return position % 4 == 0;
        case ArticulationStyle::Normal:
            return false;
        case ArticulationStyle::AlternatingStaccato:
            return position % 2 == 0;
        case ArticulationStyle::OffbeatAccent:
            return position % 2 != 0;
        case ArticulationStyle::Custom:
            return false;
    }
    return false;
}

void PatternTransformer::applySwingFeel(std::vector<Note>& notes) {
    for (size_t i = 0; i < notes.size(); i += 2) {
        if (i + 1 < notes.size()) {
            // Adjust timing for swing feel
            double swingAmount = 0.33; // Adjustable swing amount
            double beatDuration = currentGridSize;
            
            // First note of pair slightly longer
            notes[i].duration = beatDuration * (1.0 + swingAmount);
            
            // Second note of pair slightly shorter and delayed
            notes[i + 1].startTime += beatDuration * swingAmount;
            notes[i + 1].duration = beatDuration * (1.0 - swingAmount);
        }
    }
}

std::vector<Note> PatternTransformer::applyRhythmSteps(
    const std::vector<Note>& input,
    const std::vector<RhythmStep>& steps)
{
    std::vector<Note> result;
    double currentTime = 0.0;
    size_t inputIndex = 0;
    
    for (const auto& step : steps) {
        if (!step.isRest && inputIndex < input.size()) {
            Note newNote = input[inputIndex++];
            newNote.startTime = currentTime;
            newNote.duration = step.duration * currentGridSize;
            newNote.velocity = 64 + (step.accent * 21); // Base velocity + accent boost (0, 21, or 42)
            result.push_back(newNote);
        }
        currentTime += step.duration * currentGridSize;
    }
    
    return result;
}

std::vector<RhythmStep> PatternTransformer::createSyncopatedPattern(int length, bool isThreeTwoClave)
{
    std::vector<RhythmStep> steps;
    steps.reserve(length);

    // Create the basic pattern
    for (int i = 0; i < length; ++i) {
        RhythmStep step;
        step.duration = 1.0;
        step.accent = 0;
        step.active = true;
        steps.push_back(step);
    }

    // Apply syncopation based on the pattern type
    if (isThreeTwoClave) {
        // 3-2 clave pattern
        if (length >= 5) {
            steps[0].accent = 2;
            steps[2].accent = 1;
            steps[4].accent = 1;
            steps[5].accent = 2;
            steps[7].accent = 2;
        }
    } else {
        // 2-3 clave pattern
        if (length >= 5) {
            steps[0].accent = 2;
            steps[2].accent = 2;
            steps[5].accent = 2;
            steps[7].accent = 1;
            steps[9].accent = 1;
        }
    }

    return steps;
}

void PatternTransformer::applyRhythmAndArticulation(Pattern& pattern)
{
    std::vector<Note>& notes = pattern.getNotes();
    
    // Apply rhythm pattern
    notes = applyRhythmPattern(notes, currentRhythm);
    
    // Apply articulation style
    notes = applyArticulationStyle(notes, currentArticulation);
    
    // Apply swing if needed
    if (currentRhythm == RhythmPattern::Swing) {
        applySwingFeel(notes);
    }
}

std::vector<Note> PatternTransformer::applyArticulationStyle(
    const std::vector<Note>& input,
    ArticulationStyle style)
{
    std::vector<Note> notes = input;
    for (size_t i = 0; i < notes.size(); ++i) {
        notes[i].isStaccato = shouldBeStaccato(i, style);
        
        if (notes[i].isStaccato) {
            // For staccato, make the note shorter but keep the same start time
            double originalDuration = notes[i].duration;
            notes[i].duration = originalDuration * 0.5; // 50% of original duration
        }
    }
    
    return notes;
}

std::vector<Note> PatternTransformer::applyRhythmPattern(
    const std::vector<Note>& input,
    RhythmPattern pattern)
{
    std::vector<int> accents;
    std::vector<double> durations;
    
    switch (pattern) {
        case RhythmPattern::Regular:
            accents = {2, 0, 1, 0};
            durations = {1.0, 1.0, 1.0, 1.0};
            break;
            
        case RhythmPattern::Dotted:
            accents = {2, 0};
            durations = {1.5, 0.5};
            break;
            
        case RhythmPattern::Swing:
            accents = {2, 0};
            durations = {1.67, 0.33};
            break;
            
        case RhythmPattern::Syncopated:
            accents = {2, 0, 1, 0, 1, 2, 0, 1};
            durations = {1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 0.5, 0.5};
            break;
            
        case RhythmPattern::Random: {
            const int patternLength = 8;
            std::uniform_int_distribution<> accentDist(0, 2);
            std::uniform_real_distribution<> durationDist(0.5, 1.5);
            
            for (int i = 0; i < patternLength; ++i) {
                accents.push_back(accentDist(rng));
                durations.push_back(durationDist(rng));
            }
            break;
        }
            
        case RhythmPattern::Clave:
            if (isThreeTwoClave) {
                accents = {2, 0, 0, 2, 0, 0, 2, 0, 2, 0, 2, 0};
                durations = {1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5};
            } else {
                accents = {2, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2};
                durations = {1.0, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0};
            }
            break;

        case RhythmPattern::LongShort:
            accents = {2, 1};
            durations = {1.5, 0.5};
            break;

        case RhythmPattern::ShortLong:
            accents = {1, 2};
            durations = {0.5, 1.5};
            break;

        case RhythmPattern::LongShortShort:
            accents = {2, 1, 1};
            durations = {1.5, 0.25, 0.25};
            break;

        case RhythmPattern::ShortShortLong:
            accents = {1, 1, 2};
            durations = {0.25, 0.25, 1.5};
            break;

        case RhythmPattern::DottedEighth:
            accents = {2, 1};
            durations = {1.5, 0.5};
            break;

        case RhythmPattern::Triplet:
            accents = {2, 1, 1};
            durations = {0.33, 0.33, 0.33};
            break;

        case RhythmPattern::Straight:
            accents = {2, 1, 2, 1};
            durations = {1.0, 1.0, 1.0, 1.0};
            break;

        case RhythmPattern::ThreeTwoClave:
            accents = {2, 0, 2, 0, 2, 0, 0, 2, 0, 2};
            durations = {1.0, 0.5, 1.0, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 1.0};
            break;

        case RhythmPattern::TwoThreeClave:
            accents = {2, 0, 2, 0, 0, 2, 0, 2, 0, 2};
            durations = {1.0, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 1.0, 0.5, 1.0};
            break;

        case RhythmPattern::Shuffle:
            accents = {2, 1, 2, 1};
            durations = {0.75, 0.25, 0.75, 0.25};
            break;

        case RhythmPattern::Custom:
            // Use default pattern for custom
            accents = {2, 1, 2, 1};
            durations = {1.0, 1.0, 1.0, 1.0};
            break;
    }
    
    auto steps = createSyncopatedPattern(accents.size(), isThreeTwoClave);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applySambaPattern(const std::vector<Note>& input)
{
    std::vector<int> accents = {2, 0, 1, 0, 2, 0, 1, 0};
    auto steps = createSyncopatedPattern(accents.size(), true);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyBossaNovaPattern(const std::vector<Note>& input)
{
    std::vector<int> accents = {2, 0, 1, 0, 2, 0, 1, 0};
    auto steps = createSyncopatedPattern(accents.size(), true);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyRumbaPattern(const std::vector<Note>& input)
{
    std::vector<int> accents = {2, 0, 1, 1, 2, 0, 1, 0};
    auto steps = createSyncopatedPattern(accents.size(), true);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyMamboPattern(const std::vector<Note>& input)
{
    std::vector<int> accents = {2, 1, 0, 1, 2, 1, 0, 1};
    auto steps = createSyncopatedPattern(accents.size(), true);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyChaChaPattern(const std::vector<Note>& input)
{
    std::vector<int> accents = {2, 0, 1, 0, 2, 0, 1, 0};
    auto steps = createSyncopatedPattern(accents.size(), true);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyClavePattern(const std::vector<Note>& input, bool isThreeTwo)
{
    std::vector<int> accents;
    if (isThreeTwo) {
        // 3-2 clave pattern
        accents = {2, 0, 2, 0, 2, 0, 0, 2, 0, 2};
    } else {
        // 2-3 clave pattern
        accents = {2, 0, 2, 0, 0, 2, 0, 2, 0, 2};
    }
    auto steps = createSyncopatedPattern(accents.size(), isThreeTwo);
    return applyRhythmSteps(input, steps);
}

void PatternTransformer::logTransformationStart(TransformationType type, const std::vector<Note>& input) const {
    PTLogger::log(LogLevel::Info, 
                 "Starting transformation: " + PTLogger::transformationTypeToString(type) + 
                 " with input: " + PTLogger::notesToString(input),
                 __func__);
}

void PatternTransformer::logTransformationEnd(TransformationType type, const std::vector<Note>& output) const {
    PTLogger::log(LogLevel::Info, 
                 "Completed transformation: " + PTLogger::transformationTypeToString(type) + 
                 " with output: " + PTLogger::notesToString(output),
                 __func__);
}

void PatternTransformer::logPatternGeneration(const std::vector<Note>& result, int targetLength) const {
    PTLogger::log(LogLevel::Info, 
                 "Generated pattern with " + std::to_string(result.size()) + 
                 " notes (target length: " + std::to_string(targetLength) + ")",
                 __func__);
}

void PatternTransformer::logRhythmApplication(const std::vector<Note>& input, const std::vector<RhythmStep>& steps) const {
    std::stringstream ss;
    ss << "Applying rhythm to " << input.size() << " notes with " << steps.size() << " rhythm steps";
    PTLogger::log(LogLevel::Info, ss.str(), __func__);
}

std::vector<Note> PatternTransformer::generatePattern(int targetLength)
{
    std::vector<Note> result;
    result.reserve(static_cast<size_t>(targetLength));
    
    // If we have seed notes, use them as a basis
    if (!seedNotes.empty()) {
        result = seedNotes;
    } else {
        // Generate a simple pattern based on the scale
        Note note;
        note.pitch = currentScale.root;
        note.startTime = 0.0f;
        note.duration = 1.0f;
        result.push_back(note);
    }
    
    // Extend pattern to desired length
    while (result.size() < static_cast<size_t>(targetLength)) {
        Note newNote = result.back();
        newNote.startTime += newNote.duration;
        
        // Randomly choose next pitch from scale
        int steps = getRandomInt(-2, 2);
        newNote.pitch = getNextScaleNote(newNote.pitch, steps);
        
        result.push_back(newNote);
    }
    
    // Trim to exact length if needed
    if (result.size() > static_cast<size_t>(targetLength)) {
        result.resize(targetLength);
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyTransformation(const std::vector<Note>& input, TransformationType type)
{
    logTransformationStart(type, input);
    
    std::vector<Note> result;
    switch (type) {
        case TransformationType::StepUp:
            result = applyStepUp(input);
            break;
        case TransformationType::StepDown:
            result = applyStepDown(input);
            break;
        case TransformationType::UpTwoDownOne:
            result = applyUpTwoDownOne(input);
            break;
        case TransformationType::SkipOne:
            result = applySkipOne(input);
            break;
        case TransformationType::Arch:
            result = applyArch(input);
            break;
        case TransformationType::Pendulum:
            result = applyPendulum(input);
            break;
        case TransformationType::PowerChord:
            result = applyPowerChord(input);
            break;
        case TransformationType::RandomFree:
            result = applyRandomFree(input);
            break;
        case TransformationType::RandomInKey:
            result = applyRandomInKey(input);
            break;
        case TransformationType::RandomRhythmic:
            result = applyRandomRhythmic(input);
            break;
        case TransformationType::Invert:
            result = applyInversion(input);
            break;
        case TransformationType::Mirror:
            result = applyMirror(input);
            break;
        case TransformationType::Retrograde:
            result = applyRetrograde(input);
            break;
    }
    
    logTransformationEnd(type, result);
    return result;
}

int PatternTransformer::getRandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

double PatternTransformer::getRandomDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}