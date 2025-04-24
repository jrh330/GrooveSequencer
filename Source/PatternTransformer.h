#pragma once

#include <JuceHeader.h>
#include "Pattern.h"
#include "Common.h"
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <mutex>
#include <sstream>

namespace PTLogger {
    void log(LogLevel level, const std::string& message, const std::string& function);
    std::string noteToString(const Note& note);
    std::string notesToString(const std::vector<Note>& notes);
    std::string rhythmPatternToString(RhythmPattern pattern);
    std::string transformationTypeToString(TransformationType type);
    std::string articulationStyleToString(ArticulationStyle style);
}

// Scale definition for melodic patterns
struct Scale {
    int root;
    std::vector<int> intervals;
};

// Rhythm step definition for detailed rhythm control
struct RhythmStep {
    double duration = 1.0;  // Duration relative to grid size
    int accent = 0;        // 0 = no accent, 1 = medium, 2 = strong
    bool active = true;    // Whether this step produces a note or is a rest
    bool isRest = false;   // Whether this step is a rest
    
    RhythmStep() = default;
    
    RhythmStep(double d, int a, bool act, bool rest = false) 
        : duration(d)
        , accent(std::clamp(a, 0, 2))
        , active(act)
        , isRest(rest)
    {
        validateDuration();
    }
    
    bool operator==(const RhythmStep& other) const {
        return duration == other.duration && 
               accent == other.accent && 
               active == other.active &&
               isRest == other.isRest;
    }
    
    bool operator!=(const RhythmStep& other) const {
        return !(*this == other);
    }
    
    [[nodiscard]] bool isValid() const {
        return duration > 0.0 && accent >= 0 && accent <= 2;
    }
    
    [[nodiscard]] juce::var toVar() const {
        auto obj = std::make_unique<juce::DynamicObject>();
        obj->setProperty("duration", duration);
        obj->setProperty("accent", accent);
        obj->setProperty("active", active);
        obj->setProperty("isRest", isRest);
        return juce::var(obj.release());
    }
    
    static RhythmStep fromVar(const juce::var& v) {
        RhythmStep step;
        if (auto* obj = v.getDynamicObject()) {
            step.duration = obj->getProperty("duration");
            step.accent = obj->getProperty("accent");
            step.active = obj->getProperty("active");
            step.isRest = obj->getProperty("isRest");
            step.validateDuration();
        }
        return step;
    }

private:
    void validateDuration() {
        if (duration <= 0.0) duration = 1.0;
    }
};

struct RandomParameters {
    // Pitch randomization
    int minPitchOffset = -12;
    int maxPitchOffset = 12;
    
    // Scale-based randomization
    int minScaleSteps = -3;
    int maxScaleSteps = 3;
    
    // Rhythm randomization
    double minDurationMultiplier = 0.5;
    double maxDurationMultiplier = 2.0;
    
    // Probability controls
    double restProbability = 0.0;      // Probability of generating a rest
    double repeatProbability = 0.0;    // Probability of repeating last note
    double octaveJumpProbability = 0.0; // Probability of jumping octaves
};

class PatternTransformer {
public:
    PatternTransformer();
    
    // Prevent copying to avoid RNG state duplication
    PatternTransformer(const PatternTransformer&) = delete;
    PatternTransformer& operator=(const PatternTransformer&) = delete;
    
    // Move operations are deleted due to std::mutex
    PatternTransformer(PatternTransformer&&) noexcept = delete;
    PatternTransformer& operator=(PatternTransformer&&) noexcept = delete;
    
    // Pattern generation and transformation
    void setSeedNotes(const std::vector<Note>& seeds);
    [[nodiscard]] Pattern generatePattern(TransformationType type, int length);
    [[nodiscard]] Pattern transformPattern(const Pattern& source, TransformationType type);
    [[nodiscard]] std::vector<Note> previewTransformation(TransformationType type, int previewLength);
    [[nodiscard]] std::vector<Note> generatePattern(int targetLength);
    [[nodiscard]] std::vector<Note> applyTransformation(const std::vector<Note>& input, TransformationType type);
    
    // Scale and rhythm settings
    void setScale(const Scale& scale);
    [[nodiscard]] const Scale& getScale() const { return currentScale; }
    void setRhythmPattern(RhythmPattern pattern);
    [[nodiscard]] RhythmPattern getRhythmPattern() const { return currentRhythm; }
    void setArticulationStyle(ArticulationStyle style);
    [[nodiscard]] ArticulationStyle getArticulationStyle() const { return currentArticulation; }
    void setGridSize(double size);
    [[nodiscard]] double getGridSize() const { return currentGridSize; }
    
    // Random parameters
    void setRandomParameters(const RandomParameters& params);
    [[nodiscard]] const RandomParameters& getRandomParameters() const { return randomParams; }
    
    // Combined pattern generation
    [[nodiscard]] Pattern generatePatternWithRhythm(
        TransformationType type, 
        RhythmPattern rhythm, 
        ArticulationStyle articulation, 
        int length);
    
    // Pattern application methods
    [[nodiscard]] Pattern applyRhythmAndArticulation(
        const Pattern& source, 
        TransformationType type,
        RhythmPattern rhythm, 
        ArticulationStyle style, 
        int length);

    /**
     * @brief Applies rhythm steps to a sequence of notes
     * @param input The input notes to apply rhythm to
     * @param steps The rhythm steps to apply
     * @return The transformed notes with applied rhythm
     * @throws std::invalid_argument if input or steps are invalid
     */
    [[nodiscard]] std::vector<Note> applyRhythmSteps(
        const std::vector<Note>& input,
        const std::vector<RhythmStep>& steps);
    
private:
    // Member variables
    std::vector<Note> seedNotes;
    Scale currentScale;
    RhythmPattern currentRhythm;
    ArticulationStyle currentArticulation;
    double currentGridSize;
    RandomParameters randomParams;
    bool isThreeTwoClave;
    
    // Thread-safe RNG
    mutable std::mt19937 rng;
    mutable std::mutex rngMutex;
    
    // Basic transformations
    [[nodiscard]] std::vector<Note> applyStepUp(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyStepDown(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyUpTwoDownOne(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applySkipOne(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyAlternateOctave(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyInversion(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyMirror(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyRetrograde(const std::vector<Note>& input);
    
    // Random transformations
    [[nodiscard]] std::vector<Note> applyRandomFree(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyRandomInKey(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyRandomRhythmic(const std::vector<Note>& input);
    
    // Pattern transformations
    [[nodiscard]] std::vector<Note> applyArch(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyPendulum(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyPowerChord(const std::vector<Note>& input);
    
    // Rhythm methods
    [[nodiscard]] std::vector<Note> applyRhythmPattern(const std::vector<Note>& input, RhythmPattern pattern);
    [[nodiscard]] std::vector<Note> applyArticulationStyle(const std::vector<Note>& input, ArticulationStyle style);
    [[nodiscard]] std::vector<RhythmStep> createSyncopatedPattern(const std::vector<int>& accents, const std::vector<double>& durations);
    [[nodiscard]] double calculateNoteDuration(int position, RhythmPattern pattern);
    [[nodiscard]] bool shouldBeStaccato(int position, ArticulationStyle style);
    void applySwingFeel(std::vector<Note>& notes);
    
    // Latin rhythm patterns
    [[nodiscard]] std::vector<Note> applySambaPattern(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyBossaNovaPattern(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyRumbaPattern(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyMamboPattern(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyChaChaPattern(const std::vector<Note>& input);
    [[nodiscard]] std::vector<Note> applyClavePattern(const std::vector<Note>& input, bool isThreeTwo);
    
    // Utility methods
    [[nodiscard]] int getNextScaleNote(int currentPitch, int steps);
    [[nodiscard]] int snapToScale(int pitch);
    [[nodiscard]] Note createNote(int pitch, double startTime, double duration, int velocity = 100);
    [[nodiscard]] double getRandomDouble(double min, double max);
    [[nodiscard]] int getRandomInt(int min, int max);

    // Add logging helpers
    void logTransformationStart(TransformationType type, const std::vector<Note>& input) const;
    void logTransformationEnd(TransformationType type, const std::vector<Note>& output) const;
    void logPatternGeneration(const std::vector<Note>& result, int targetLength) const;
    void logRhythmApplication(const std::vector<Note>& input, const std::vector<RhythmStep>& steps) const;
}; 