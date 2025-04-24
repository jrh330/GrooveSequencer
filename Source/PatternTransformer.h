#pragma once

#include <vector>
#include <string>
#include <memory>
#include <random>
#include "Pattern.h"

// Forward declarations
class GrooveSequencerAudioProcessor;

// Scale definition for melodic patterns
struct Scale {
    int root;
    std::vector<int> intervals;
};

// Rhythm step definition for detailed rhythm control
struct RhythmStep {
    double duration{1.0};  // Duration in grid units
    int accent{0};      // 0 = none, 1 = medium, 2 = strong
    bool isRest{false}; // Whether this step is a rest
    
    RhythmStep() = default;
    RhythmStep(double d, int a, bool r) : duration(d), accent(a), isRest(r) {}
};

// Rhythm patterns
enum class RhythmPattern {
    Regular,
    LongShort,
    ShortLong,
    LongShortShort,
    ShortShortLong,
    DottedEighth,
    Triplet,
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
    Pattern,
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
    float restProbability{0.1f};
    float repeatProbability{0.2f};
    float octaveJumpProbability{0.1f};
    int minPitchOffset{-4};
    int maxPitchOffset{4};
    int minScaleSteps{-2};
    int maxScaleSteps{2};
    float minDurationMultiplier{0.5f};
    float maxDurationMultiplier{2.0f};
};

class PatternTransformer {
public:
    PatternTransformer();
    
    // Pattern generation and transformation
    void setSeedNotes(const std::vector<Note>& seeds);
    Pattern generatePattern(TransformationType type, int length);
    Pattern transformPattern(const Pattern& source, TransformationType type);
    std::vector<Note> previewTransformation(TransformationType type, int previewLength);
    
    // Scale and rhythm settings
    void setScale(const Scale& scale);
    void setRhythmPattern(RhythmPattern pattern);
    void setArticulationStyle(ArticulationStyle style);
    void setGridSize(double size);
    
    // Random parameters
    void setRandomParameters(const RandomParameters& params);
    RandomParameters getRandomParameters() const;
    
    // Combined pattern generation
    Pattern generatePatternWithRhythm(TransformationType type, RhythmPattern rhythm, 
                                    ArticulationStyle articulation, int length);
    
    // Pattern application methods
    Pattern applyRhythmAndArticulation(const Pattern& source, TransformationType type,
                                     RhythmPattern rhythm, ArticulationStyle style, int length);
    
private:
    // Basic transformations
    std::vector<Note> applyStepUp(const std::vector<Note>& input);
    std::vector<Note> applyStepDown(const std::vector<Note>& input);
    std::vector<Note> applyUpTwoDownOne(const std::vector<Note>& input);
    std::vector<Note> applySkipOne(const std::vector<Note>& input);
    std::vector<Note> applyAlternateOctave(const std::vector<Note>& input);
    std::vector<Note> applyInversion(const std::vector<Note>& input);
    std::vector<Note> applyMirror(const std::vector<Note>& input);
    std::vector<Note> applyRetrograde(const std::vector<Note>& input);
    
    // Random transformations
    std::vector<Note> applyRandomFree(const std::vector<Note>& input);
    std::vector<Note> applyRandomInKey(const std::vector<Note>& input);
    std::vector<Note> applyRandomRhythmic(const std::vector<Note>& input);
    
    // Pattern transformations
    std::vector<Note> applyArch(const std::vector<Note>& input);
    std::vector<Note> applyPendulum(const std::vector<Note>& input);
    std::vector<Note> applyPowerChord(const std::vector<Note>& input);
    
    // Rhythm methods
    std::vector<Note> applyBasicRhythmPattern(const std::vector<Note>& input, RhythmPattern pattern);
    std::vector<Note> applyRhythmPattern(const std::vector<Note>& input, RhythmPattern pattern);
    std::vector<Note> applyArticulationStyle(const std::vector<Note>& input, ArticulationStyle style);
    std::vector<Note> applyRhythmSteps(const std::vector<Note>& input, const std::vector<RhythmStep>& steps);
    std::vector<RhythmStep> createSyncopatedPattern(const std::vector<int>& accents, const std::vector<double>& durations);
    double calculateNoteDuration(int position, RhythmPattern pattern);
    bool shouldBeStaccato(int position, ArticulationStyle style);
    void applySwingFeel(std::vector<Note>& notes);
    
    // Latin rhythm patterns
    std::vector<Note> applySambaPattern(const std::vector<Note>& input);
    std::vector<Note> applyBossaNovaPattern(const std::vector<Note>& input);
    std::vector<Note> applyRumbaPattern(const std::vector<Note>& input);
    std::vector<Note> applyMamboPattern(const std::vector<Note>& input);
    std::vector<Note> applyChaChaPattern(const std::vector<Note>& input);
    std::vector<Note> applyClavePattern(const std::vector<Note>& input, bool isThreeTwo);
    
    // Scale utilities
    int getNextScaleNote(int currentPitch, int steps);
    int snapToScale(int pitch);
    Note createNote(int pitch, double startTime, double duration, int velocity = 100);
    
    // Member variables
    std::vector<Note> seedNotes;
    Scale currentScale;
    RhythmPattern currentRhythm;
    ArticulationStyle currentArticulation;
    double currentGridSize;
    bool isThreeTwoClave;
    RandomParameters randomParams;
    std::mt19937 rng;
}; 