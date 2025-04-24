#pragma once

#include <vector>
#include <string>
#include <memory>
#include <random>
#include "PluginProcessor.h"

// Scale definition for melodic patterns
struct Scale {
    int root;
    std::vector<int> intervals;
};

// Rhythm step definition for detailed rhythm control
struct RhythmStep {
    double duration;
    int accent;
    bool isRest;
};

// Rhythm patterns
enum class RhythmPattern {
    Regular,
    Dotted,
    Swing,
    Syncopated,
    Random,
    Clave
};

// Articulation styles
enum class ArticulationStyle {
    Legato,
    Staccato,
    Mixed,
    Accented,
    Random
};

// Types of pattern transformations
enum class TransformationType {
    StepUp,
    StepDown,
    UpTwoDownOne,
    SkipOne,
    Arch,
    Pendulum,
    PowerChord,
    RandomFree,
    RandomInKey,
    RandomRhythmic,
    Invert,
    Mirror,
    Retrograde
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
    
    void setSeedNotes(const std::vector<Note>& seeds);
    Pattern generatePattern(TransformationType type, int length);
    Pattern transformPattern(const Pattern& source, TransformationType type);
    std::vector<Note> previewTransformation(TransformationType type, int previewLength);
    
    // Rhythm and articulation control
    void setRhythmPattern(RhythmPattern pattern);
    void setArticulationStyle(ArticulationStyle style);
    void setGridSize(double size);
    
    // New methods for rhythm and articulation
    Pattern applyRhythmAndArticulation(
        const Pattern& source,
        TransformationType type,
        RhythmPattern rhythm,
        ArticulationStyle articulation,
        int length);

    // Random parameter controls
    void setRandomParameters(const RandomParameters& params);
    RandomParameters getRandomParameters() const;

private:
    std::vector<Note> seedNotes;
    Scale currentScale;
    std::mt19937 rng;
    RhythmPattern currentRhythm;
    ArticulationStyle currentArticulation;
    double currentGridSize;
    bool isThreeTwoClave;
    RandomParameters randomParams;
    
    // Helper methods for transformations
    std::vector<Note> applyStepUp(const std::vector<Note>& input);
    std::vector<Note> applyStepDown(const std::vector<Note>& input);
    std::vector<Note> applyUpTwoDownOne(const std::vector<Note>& input);
    std::vector<Note> applySkipOne(const std::vector<Note>& input);
    std::vector<Note> applyArch(const std::vector<Note>& input);
    std::vector<Note> applyPendulum(const std::vector<Note>& input);
    std::vector<Note> applyPowerChord(const std::vector<Note>& input);
    std::vector<Note> applyRandomFree(const std::vector<Note>& input);
    std::vector<Note> applyRandomInKey(const std::vector<Note>& input);
    std::vector<Note> applyRandomRhythmic(const std::vector<Note>& input);
    std::vector<Note> applyInversion(const std::vector<Note>& input);
    std::vector<Note> applyMirror(const std::vector<Note>& input);
    std::vector<Note> applyRetrograde(const std::vector<Note>& input);
    std::vector<Note> applyAlternateOctave(const std::vector<Note>& input);
    
    // Helper methods for rhythm and articulation
    double calculateNoteDuration(int position, RhythmPattern pattern);
    bool shouldBeStaccato(int position, ArticulationStyle style);
    std::vector<Note> applyRhythmPattern(const std::vector<Note>& input, RhythmPattern pattern);
    std::vector<Note> applyArticulationStyle(const std::vector<Note>& input, ArticulationStyle style);
    std::vector<Note> applyClavePattern(const std::vector<Note>& input, bool isThreeTwo);
    std::vector<Note> applyRhythmSteps(const std::vector<Note>& input, const std::vector<RhythmStep>& steps);
    std::vector<RhythmStep> createSyncopatedPattern(const std::vector<int>& accents, const std::vector<double>& durations);
    
    // Latin pattern implementations
    std::vector<Note> applySambaPattern(const std::vector<Note>& input);
    std::vector<Note> applyBossaNovaPattern(const std::vector<Note>& input);
    std::vector<Note> applyRumbaPattern(const std::vector<Note>& input);
    std::vector<Note> applyMamboPattern(const std::vector<Note>& input);
    std::vector<Note> applyChaChaPattern(const std::vector<Note>& input);
    
    // Scale helpers
    int getNextScaleNote(int currentPitch, int steps);
    int snapToScale(int pitch);
    Note createNote(int pitch, double startTime, double duration, int velocity);
}; 