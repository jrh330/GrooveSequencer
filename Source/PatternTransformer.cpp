#include "PatternTransformer.h"
#include <ctime>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>

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
    , isThreeTwoClave(false)
    , rng(std::time(nullptr))
{
    // Initialize default scale (C major)
    currentScale.root = 60; // Middle C
    currentScale.intervals = {0, 2, 4, 5, 7, 9, 11}; // Major scale intervals
}

void PatternTransformer::setSeedNotes(const std::vector<Note>& seeds) {
    seedNotes = seeds;
}

Pattern PatternTransformer::generatePattern(TransformationType type, int length)
{
    Pattern result;
    result.length = length;
    
    // Generate initial notes
    std::vector<Note> notes = generatePattern(length);
    
    // Apply transformation
    notes = applyTransformation(notes, type);
    
    // Apply rhythm pattern
    notes = applyRhythmPattern(notes, currentRhythm);
    
    // Apply articulation
    notes = applyArticulationStyle(notes, currentArticulation);
    
    result.notes = std::move(notes);
    return result;
}

Pattern PatternTransformer::transformPattern(const Pattern& source, TransformationType type)
{
    Pattern result = source;
    result.notes = applyTransformation(source.notes, type);
    return result;
}

std::vector<Note> PatternTransformer::previewTransformation(TransformationType type, int previewLength) {
    Pattern preview = generatePattern(type, previewLength);
    return preview.notes;
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

std::vector<Note> PatternTransformer::applySkipOne(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    newNote.startTime = lastNote.startTime + (lastNote.duration * 2); // Skip one position
    result.push_back(newNote);
    
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

std::vector<Note> PatternTransformer::applyInversion(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    if (input.empty()) return result;
    
    // Find the center pitch
    int centerPitch = input[0].pitch;
    
    // Invert around the center pitch
    for (Note& note : result) {
        int distance = note.pitch - centerPitch;
        note.pitch = centerPitch - distance;
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyMirror(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    
    // Add reversed copy of the input
    double lastTime = input.back().startTime + input.back().duration;
    for (auto it = input.rbegin(); it != input.rend(); ++it) {
        Note newNote = *it;
        newNote.startTime = lastTime;
        lastTime += newNote.duration;
        result.push_back(newNote);
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyRetrograde(const std::vector<Note>& input) {
    std::vector<Note> result;
    double currentTime = 0.0;
    
    // Reverse the pattern
    for (auto it = input.rbegin(); it != input.rend(); ++it) {
        Note newNote = *it;
        newNote.startTime = currentTime;
        currentTime += newNote.duration;
        result.push_back(newNote);
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
std::vector<Note> PatternTransformer::applyArch(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Determine if we're in ascending or descending phase
    bool ascending = true;
    if (input.size() >= 2) {
        Note secondLastNote = input[input.size() - 2];
        if (lastNote.pitch > secondLastNote.pitch && lastNote.pitch >= input[0].pitch + 12) {
            ascending = false; // Start descending if we've gone up an octave
        } else if (lastNote.pitch < secondLastNote.pitch && lastNote.pitch <= input[0].pitch) {
            ascending = true; // Start ascending if we've returned to start
        } else {
            ascending = lastNote.pitch >= secondLastNote.pitch;
        }
    }
    
    newNote.pitch += ascending ? 2 : -2;
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyPendulum(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Determine swing direction
    bool swingRight = true;
    if (input.size() >= 2) {
        Note secondLastNote = input[input.size() - 2];
        swingRight = lastNote.pitch <= secondLastNote.pitch;
    }
    
    // Alternate between root note and higher/lower notes
    if (input.size() % 2 == 0) {
        newNote.pitch = input[0].pitch; // Return to root
    } else {
        newNote.pitch = input[0].pitch + (swingRight ? 4 : -4); // Swing up or down
    }
    
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyPowerChord(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    double nextStartTime = lastNote.startTime + lastNote.duration;
    
    // Create power chord (root + fifth)
    Note rootNote = lastNote;
    rootNote.startTime = nextStartTime;
    Note fifthNote = rootNote;
    fifthNote.pitch += 7; // Perfect fifth
    
    result.push_back(rootNote);
    result.push_back(fifthNote);
    
    return result;
}

void PatternTransformer::setRandomParameters(const RandomParameters& params) {
    randomParams = params;
}

std::vector<Note> PatternTransformer::applyRandomFree(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Check for rest
    std::uniform_real_distribution<double> probDist(0.0, 1.0);
    if (probDist(rng) < randomParams.restProbability) {
        newNote.isRest = true;
    }
    // Check for repeat
    else if (probDist(rng) < randomParams.repeatProbability) {
        // Keep the same pitch
    }
    else {
        // Random pitch within configured range
        std::uniform_int_distribution<int> pitchDist(
            randomParams.minPitchOffset,
            randomParams.maxPitchOffset
        );
        newNote.pitch += pitchDist(rng);
        
        // Check for octave jump
        if (probDist(rng) < randomParams.octaveJumpProbability) {
            newNote.pitch += (probDist(rng) < 0.5 ? -12 : 12);
        }
    }
    
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    return result;
}

std::vector<Note> PatternTransformer::applyRandomInKey(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Check for rest
    std::uniform_real_distribution<double> probDist(0.0, 1.0);
    if (probDist(rng) < randomParams.restProbability) {
        newNote.isRest = true;
    }
    // Check for repeat
    else if (probDist(rng) < randomParams.repeatProbability) {
        // Keep the same pitch
    }
    else {
        // Random step within configured scale range
        std::uniform_int_distribution<int> stepDist(
            randomParams.minScaleSteps,
            randomParams.maxScaleSteps
        );
        newNote.pitch = getNextScaleNote(lastNote.pitch, stepDist(rng));
        
        // Check for octave jump
        if (probDist(rng) < randomParams.octaveJumpProbability) {
            newNote.pitch += (probDist(rng) < 0.5 ? -12 : 12);
        }
    }
    
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    return result;
}

std::vector<Note> PatternTransformer::applyRandomRhythmic(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Check for rest
    std::uniform_real_distribution<double> probDist(0.0, 1.0);
    if (probDist(rng) < randomParams.restProbability) {
        newNote.isRest = true;
    }
    
    // Random duration variations within configured range
    std::uniform_real_distribution<double> durationDist(
        randomParams.minDurationMultiplier,
        randomParams.maxDurationMultiplier
    );
    newNote.duration *= durationDist(rng);
    newNote.startTime = lastNote.startTime + lastNote.duration;
    
    result.push_back(newNote);
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

Pattern PatternTransformer::generatePatternWithRhythm(
    TransformationType type,
    RhythmPattern rhythm,
    ArticulationStyle articulation,
    int length)
{
    // First generate the basic pattern
    Pattern pattern = generatePattern(type, length);
    
    // Apply rhythm pattern
    pattern.notes = applyRhythmPattern(pattern.notes, rhythm);
    
    // Apply articulation
    pattern.notes = applyArticulationStyle(pattern.notes, articulation);
    
    // Set the grid size
    pattern.gridSize = currentGridSize;
    
    return pattern;
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

std::vector<RhythmStep> PatternTransformer::createSyncopatedPattern(
    const std::vector<int>& accents,
    const std::vector<double>& durations)
{
    std::vector<RhythmStep> steps;
    for (size_t i = 0; i < accents.size(); ++i) {
        RhythmStep step;
        step.duration = durations[i];
        step.accent = accents[i];
        step.isRest = (accents[i] == 0 && std::uniform_real_distribution<>(0.0, 1.0)(rng) < randomParams.restProbability);
        steps.push_back(step);
    }
    return steps;
}

Pattern PatternTransformer::applyRhythmAndArticulation(
    const Pattern& source,
    TransformationType type,
    RhythmPattern rhythm,
    ArticulationStyle articulation,
    int length)
{
    // First apply the transformation
    Pattern transformed = transformPattern(source, type);
    
    // Then apply rhythm pattern
    std::vector<Note> rhythmicNotes = applyRhythmPattern(transformed.notes, rhythm);
    
    // Finally apply articulation
    std::vector<Note> articulatedNotes = applyArticulationStyle(rhythmicNotes, articulation);
    
    // Create final pattern
    Pattern result = transformed;
    result.notes = articulatedNotes;
    result.length = length;
    
    return result;
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
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applySambaPattern(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    std::vector<int> accents = {2, 0, 1, 0, 2, 0, 1, 0}; // 2 = strong, 1 = medium, 0 = weak
    std::vector<double> durations = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(result, steps);
}

std::vector<Note> PatternTransformer::applyBossaNovaPattern(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    std::vector<int> accents = {2, 0, 1, 0, 1, 0, 2, 0}; // Bossa Nova accent pattern
    std::vector<double> durations = {0.5, 0.25, 0.5, 0.25, 0.5, 0.25, 0.5, 0.25};
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(result, steps);
}

std::vector<Note> PatternTransformer::applyRumbaPattern(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    std::vector<int> accents = {2, 0, 1, 2, 0, 1, 0, 2}; // Rumba accent pattern
    std::vector<double> durations = {0.75, 0.25, 0.5, 0.5, 0.5, 0.25, 0.25, 0.5};
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(result, steps);
}

std::vector<Note> PatternTransformer::applyMamboPattern(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    std::vector<int> accents = {2, 1, 0, 2, 1, 0, 2, 0}; // Mambo accent pattern
    std::vector<double> durations = {0.5, 0.25, 0.25, 0.5, 0.25, 0.25, 0.5, 0.5};
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(result, steps);
}

std::vector<Note> PatternTransformer::applyChaChaPattern(const std::vector<Note>& input) {
    std::vector<Note> result = input;
    std::vector<int> accents = {2, 0, 1, 0, 2, 0, 1, 0}; // Cha-cha accent pattern
    std::vector<double> durations = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(result, steps);
}

std::vector<Note> PatternTransformer::applyClavePattern(const std::vector<Note>& input, bool isThreeTwo) {
    std::vector<int> accents;
    if (isThreeTwo) {
        // 3-2 Son clave
        // | 1 e & a | 2 e & a | 3 e & a | 4 e & a |
        // |>     a |>    >  |    >    |>        |
        accents = {2, 0, 0, 1, 2, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0};
    } else {
        // 2-3 Son clave
        // | 1 e & a | 2 e & a | 3 e & a | 4 e & a |
        // |>        |>        |>     a |>    >  |
        accents = {2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 1, 2, 0, 2, 0};
    }
    
    std::vector<double> durations(16, 0.25); // 16 sixteenth notes
    auto steps = createSyncopatedPattern(accents, durations);
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