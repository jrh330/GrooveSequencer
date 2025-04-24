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
    
    // Initialize random parameters with defaults
    randomParams.restProbability = 0.1f;
    randomParams.repeatProbability = 0.2f;
    randomParams.octaveJumpProbability = 0.1f;
    randomParams.minPitchOffset = -4;
    randomParams.maxPitchOffset = 4;
    randomParams.minScaleSteps = -2;
    randomParams.maxScaleSteps = 2;
    randomParams.minDurationMultiplier = 0.5f;
    randomParams.maxDurationMultiplier = 2.0f;
}

void PatternTransformer::setSeedNotes(const std::vector<Note>& seeds) {
    seedNotes = seeds;
}

Pattern PatternTransformer::generatePattern(TransformationType type, int length) {
    Pattern result;
    result.length = length;
    result.tempo = 120.0; // Default tempo
    result.gridSize = currentGridSize;
    
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

RandomParameters PatternTransformer::getRandomParameters() const {
    return randomParams;
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

Pattern PatternTransformer::applyRhythmAndArticulation(
    const Pattern& source,
    TransformationType type,
    RhythmPattern rhythm,
    ArticulationStyle style,
    int length)
{
    // First apply the transformation
    Pattern transformed = transformPattern(source, type);
    
    // Then apply rhythm pattern
    std::vector<Note> rhythmicNotes = applyRhythmPattern(transformed.notes, rhythm);
    
    // Finally apply articulation
    std::vector<Note> articulatedNotes = applyArticulationStyle(rhythmicNotes, style);
    
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

std::vector<Note> PatternTransformer::applyBasicRhythmPattern(
    const std::vector<Note>& input,
    RhythmPattern pattern)
{
    std::vector<Note> result = input;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i].duration = calculateNoteDuration(i, pattern);
    }
    return result;
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
            
        case ArticulationStyle::Random:
            return std::uniform_real_distribution<>(0.0, 1.0)(rng) < 0.5;
            
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

std::vector<Note> PatternTransformer::applyRhythmPattern(
    const std::vector<Note>& input,
    RhythmPattern pattern)
{
    // First apply basic rhythm pattern
    std::vector<Note> basicPattern = applyBasicRhythmPattern(input, pattern);
    
    // Then apply specific pattern modifications
    switch (pattern) {
        case RhythmPattern::Swing:
            applySwingFeel(basicPattern);
            break;
            
        case RhythmPattern::Syncopated: {
            std::vector<int> accents = {2, 0, 1, 0, 1, 2, 0, 1};
            std::vector<double> durations = {1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 0.5, 0.5};
            auto steps = createSyncopatedPattern(accents, durations);
            return applyRhythmSteps(input, steps);
        }
            
        case RhythmPattern::Clave:
            return applyClavePattern(input, isThreeTwoClave);
            
        default:
            return basicPattern;
    }
    
    return basicPattern;
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