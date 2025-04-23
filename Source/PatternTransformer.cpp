#include "PatternTransformer.h"
#include <ctime>
#include <algorithm>

PatternTransformer::PatternTransformer() 
    : rng(std::time(nullptr))
    , currentRhythm(RhythmPattern::Regular)
    , currentArticulation(ArticulationStyle::Legato)
    , currentGridSize(0.25) // Default to 16th notes
    , isThreeTwoClave(false)
{
    // Initialize with C major scale as default
    currentScale.root = 60; // Middle C
    currentScale.intervals = {0, 2, 4, 5, 7, 9, 11}; // Major scale intervals
}

void PatternTransformer::setSeedNotes(const std::vector<Note>& seeds) {
    seedNotes = seeds;
}

Pattern PatternTransformer::generatePattern(TransformationType type, int length) {
    Pattern result;
    result.length = length;
    result.tempo = 120.0; // Default tempo
    
    // Start with seed notes
    result.notes = seedNotes;
    
    // Generate additional notes based on transformation type
    while (result.notes.size() < length) {
        std::vector<Note> nextNotes;
        switch (type) {
            case TransformationType::StepUp:
                nextNotes = applyStepUp(result.notes);
                break;
            case TransformationType::StepDown:
                nextNotes = applyStepDown(result.notes);
                break;
            case TransformationType::UpTwoDownOne:
                nextNotes = applyUpTwoDownOne(result.notes);
                break;
            case TransformationType::SkipOne:
                nextNotes = applySkipOne(result.notes);
                break;
            case TransformationType::Arch:
                nextNotes = applyArch(result.notes);
                break;
            case TransformationType::Pendulum:
                nextNotes = applyPendulum(result.notes);
                break;
            case TransformationType::PowerChord:
                nextNotes = applyPowerChord(result.notes);
                break;
            case TransformationType::RandomFree:
                nextNotes = applyRandomFree(result.notes);
                break;
            case TransformationType::RandomInKey:
                nextNotes = applyRandomInKey(result.notes);
                break;
            case TransformationType::RandomRhythmic:
                nextNotes = applyRandomRhythmic(result.notes);
                break;
            default:
                break;
        }
        result.notes.insert(result.notes.end(), nextNotes.begin(), nextNotes.end());
    }
    
    // Trim to exact length if necessary
    if (result.notes.size() > length) {
        result.notes.resize(length);
    }
    
    return result;
}

Pattern PatternTransformer::transformPattern(const Pattern& source, TransformationType type) {
    Pattern result = source;
    
    switch (type) {
        case TransformationType::Invert:
            result.notes = applyInversion(source.notes);
            break;
        case TransformationType::Mirror:
            result.notes = applyMirror(source.notes);
            break;
        case TransformationType::Retrograde:
            result.notes = applyRetrograde(source.notes);
            break;
        default:
            // For other transformations, generate new pattern
            result = generatePattern(type, source.length);
            break;
    }
    
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
    int peakHeight = 7; // Maximum steps up before descending
    
    if (input.size() >= 2) {
        Note secondLastNote = input[input.size() - 2];
        if (lastNote.pitch < secondLastNote.pitch) {
            ascending = false;
        }
    }
    
    if (ascending && lastNote.pitch >= input[0].pitch + (peakHeight * 2)) {
        ascending = false;
    }
    
    newNote.pitch = getNextScaleNote(lastNote.pitch, ascending ? 2 : -2);
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyPendulum(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    int centralPitch = input[0].pitch; // Use first note as central tone
    
    // Calculate current swing distance
    int swingStep = (input.size() / 2) + 1; // Increases with each pair of notes
    
    // Alternate between central tone and ascending steps
    if (lastNote.pitch == centralPitch) {
        Note newNote = lastNote;
        newNote.pitch = getNextScaleNote(centralPitch, swingStep);
        newNote.startTime = lastNote.startTime + lastNote.duration;
        result.push_back(newNote);
    } else {
        Note newNote = lastNote;
        newNote.pitch = centralPitch;
        newNote.startTime = lastNote.startTime + lastNote.duration;
        result.push_back(newNote);
    }
    
    return result;
}

std::vector<Note> PatternTransformer::applyPowerChord(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Power chord intervals (root, fifth, octave)
    static const std::vector<int> powerChordIntervals = {0, 7, 12};
    
    // Cycle through power chord notes
    int currentPosition = 0;
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i].pitch == lastNote.pitch) {
            currentPosition = (i % powerChordIntervals.size());
            break;
        }
    }
    
    int nextPosition = (currentPosition + 1) % powerChordIntervals.size();
    int basePitch = input[0].pitch; // Use first note as root
    newNote.pitch = basePitch + powerChordIntervals[nextPosition];
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyRandomFree(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Generate random pitch between reasonable MIDI values (36-96)
    std::uniform_int_distribution<> pitchDist(36, 96);
    newNote.pitch = pitchDist(rng);
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyRandomInKey(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Choose random scale degree
    std::uniform_int_distribution<> degDist(0, currentScale.intervals.size() - 1);
    int scaleDegree = degDist(rng);
    
    // Choose random octave between 3 and 6
    std::uniform_int_distribution<> octDist(3, 6);
    int octave = octDist(rng);
    
    newNote.pitch = (octave * 12) + currentScale.intervals[scaleDegree];
    newNote.startTime = lastNote.startTime + lastNote.duration;
    result.push_back(newNote);
    
    return result;
}

std::vector<Note> PatternTransformer::applyRandomRhythmic(const std::vector<Note>& input) {
    std::vector<Note> result;
    if (input.empty()) return result;
    
    Note lastNote = input.back();
    Note newNote = lastNote;
    
    // Keep the rhythm (duration and timing) but randomize pitch in scale
    std::uniform_int_distribution<> degDist(0, currentScale.intervals.size() - 1);
    int scaleDegree = degDist(rng);
    
    // Stay within 2 octaves of the original pitch
    int baseOctave = lastNote.pitch / 12;
    std::uniform_int_distribution<> octDist(baseOctave - 1, baseOctave + 1);
    int octave = octDist(rng);
    
    newNote.pitch = (octave * 12) + currentScale.intervals[scaleDegree];
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
    applyArticulation(pattern.notes, articulation);
    
    // Set the grid size
    pattern.gridSize = currentGridSize;
    
    return pattern;
}

double PatternTransformer::calculateNoteDuration(int position, RhythmPattern pattern) {
    double baseUnit = currentGridSize;
    double longNote = baseUnit * 2.0;
    double shortNote = baseUnit * 0.5;
    
    switch (pattern) {
        case RhythmPattern::Regular:
            return baseUnit;
            
        case RhythmPattern::LongShort:
            return (position % 2 == 0) ? longNote : shortNote;
            
        case RhythmPattern::ShortLong:
            return (position % 2 == 0) ? shortNote : longNote;
            
        case RhythmPattern::LongShortShort: {
            int pos = position % 3;
            return (pos == 0) ? longNote : shortNote;
        }
            
        case RhythmPattern::ShortShortLong: {
            int pos = position % 3;
            return (pos == 2) ? longNote : shortNote;
        }
            
        case RhythmPattern::DottedEighth: {
            int pos = position % 2;
            return (pos == 0) ? baseUnit * 1.5 : baseUnit * 0.5;
        }
            
        case RhythmPattern::Triplet:
            return baseUnit * (2.0 / 3.0);
            
        case RhythmPattern::Swing: {
            int pos = position % 2;
            return (pos == 0) ? baseUnit * 1.67 : baseUnit * 0.33;
        }
            
        default:
            return baseUnit;
    }
}

bool PatternTransformer::shouldBeStaccato(int position, ArticulationStyle style) {
    switch (style) {
        case ArticulationStyle::Legato:
            return false;
            
        case ArticulationStyle::Staccato:
            return true;
            
        case ArticulationStyle::Mixed:
            return position % 2 == 0;
            
        case ArticulationStyle::Pattern: {
            // Example pattern: staccato every third note
            return position % 3 == 0;
        }
            
        default:
            return false;
    }
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
            newNote.duration = step.duration;
            newNote.accent = step.accent;
            newNote.velocity = 64 + (step.accent * 32); // Base velocity + accent boost
            result.push_back(newNote);
        }
        currentTime += step.duration;
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
        step.duration = durations[i] * currentGridSize;
        step.accent = accents[i];
        step.isRest = (accents[i] == 0);
        steps.push_back(step);
    }
    return steps;
}

std::vector<Note> PatternTransformer::applyRhythmPattern(
    const std::vector<Note>& input,
    RhythmPattern pattern)
{
    std::vector<Note> result = input;
    double currentTime = 0.0;
    
    // Create basic rhythm patterns
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
            // Generate random accents and durations
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
            // 3-2 Son clave pattern
            accents = {2, 0, 0, 2, 0, 0, 2, 0, 2, 0, 2, 0};
            durations = {1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5};
            break;
    }
    
    // Create and apply rhythm steps
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applySambaPattern(const std::vector<Note>& input) {
    // Samba basic pattern (2/4 time)
    std::vector<int> accents = {2, 0, 1, 1, 0, 1, 2, 1};
    std::vector<double> durations(8, 0.25); // Eight equal divisions
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyBossaNovaPattern(const std::vector<Note>& input) {
    // Bossa Nova basic pattern
    std::vector<int> accents = {2, 0, 1, 1, 0, 2, 1, 0};
    std::vector<double> durations(8, 0.25);
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyRumbaPattern(const std::vector<Note>& input) {
    // Rumba clave pattern
    std::vector<int> accents = {2, 0, 1, 0, 1, 0, 1, 0, 2, 0, 1, 1, 0, 0, 1, 1};
    std::vector<double> durations(16, 0.25);
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyMamboPattern(const std::vector<Note>& input) {
    // Mambo basic pattern
    std::vector<int> accents = {2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1};
    std::vector<double> durations(16, 0.25);
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
}

std::vector<Note> PatternTransformer::applyChaChaPattern(const std::vector<Note>& input) {
    // Cha-cha-cha pattern
    std::vector<int> accents = {2, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 0, 2, 0, 0, 1};
    std::vector<double> durations = {0.375, 0.125, 0.25, 0.25, 0.375, 0.125, 0.25, 0.25,
                                   0.375, 0.125, 0.25, 0.25, 0.375, 0.125, 0.25, 0.25};
    
    auto steps = createSyncopatedPattern(accents, durations);
    return applyRhythmSteps(input, steps);
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

std::vector<Note> PatternTransformer::applyArticulation(
    std::vector<Note>& notes,
    ArticulationStyle style)
{
    for (size_t i = 0; i < notes.size(); ++i) {
        notes[i].isStaccato = shouldBeStaccato(i, style);
        
        if (notes[i].isStaccato) {
            // For staccato, make the note shorter but keep the same start time
            double originalDuration = notes[i].duration;
            notes[i].duration = originalDuration * 0.5; // 50% of original duration
            
            // The remaining time becomes a rest
            // (we don't need to explicitly represent the rest)
        }
    }
    
    return notes;
} 