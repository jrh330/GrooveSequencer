#pragma once

#include <vector>
#include <string>
#include <memory>
#include <random>

struct Note {
    int pitch;
    int velocity;
    double startTime;
    double duration;
    bool isStaccato;  // New: controls note articulation
    int accent;     // 0 = normal, 1 = accented, 2 = strong accent
};

struct Pattern {
    std::vector<Note> notes;
    int length;      // in steps
    double tempo;
    double gridSize; // 1.0 = quarter note, 0.5 = eighth note, 0.25 = sixteenth note
    int timeSignatureNumerator;   // e.g., 4 for 4/4, 3 for 3/4
    int timeSignatureDenominator; // e.g., 4 for 4/4
};

// Scale definition for musical patterns
struct Scale {
    std::vector<int> intervals;  // Intervals from root note
    int root;                    // Root note MIDI number
};

// Extended rhythm pattern definition
enum class RhythmPattern {
    Regular,            // All notes same duration
    LongShort,         // Alternating long and short notes
    ShortLong,         // Alternating short and long notes
    LongShortShort,    // Long, short, short pattern
    ShortShortLong,    // Short, short, long pattern
    DottedEighth,      // Dotted eighth followed by sixteenth
    Triplet,           // Three equal notes in space of two
    Swing,             // Swing feel with longer first note
    
    // Latin patterns
    Samba,              // 2/4 Brazilian rhythm
    BossaNova,          // Modified samba with subtle accents
    Rumba,              // Cuban rhythm, 4/4
    Mambo,              // Upbeat Cuban dance rhythm
    ChaCha,             // Cuban rhythm with distinctive "cha-cha-cha"
    Tango,              // Argentine 2/4 or 4/4 rhythm
    Salsa,              // Complex 4/4 pattern with clave
    Baiao,              // Brazilian northeastern rhythm
    Beguine,            // Similar to rumba but with different accents
    Clave23,            // 2-3 Son clave rhythm
    Clave32,            // 3-2 Son clave rhythm
    Montuno,            // Cuban piano pattern
    Cascara,            // Timbale pattern used in salsa
    
    // Compound patterns
    SalsaClave,         // Salsa with clave emphasis
    MamboTumbao,        // Mambo with bass pattern
    SambaPartido        // Partido alto samba variation
};

// New: Articulation style
enum class ArticulationStyle {
    Legato,            // Notes connected
    Staccato,          // Short notes with gaps
    Mixed,             // Alternating legato and staccato
    Pattern            // Custom pattern of articulations
};

enum class TransformationType {
    StepUp,              // Move up by step
    StepDown,            // Move down by step
    UpTwoDownOne,       // Up 2 steps, down 1 step
    SkipOne,            // Play note, skip position, play note
    AlternateOctave,    // Alternate between octaves
    Invert,             // Invert the pattern
    Mirror,             // Mirror the pattern
    Retrograde,         // Play pattern backwards
    Arch,               // Goes up and then comes down in set increments
    Pendulum,           // Central tone with increasing steps up
    PowerChord,         // Uses 1, 5, 8 based on starting tone
    RandomFree,         // Completely random notes
    RandomInKey,        // Random notes within a key
    RandomRhythmic      // Random tones with fixed rhythm
};

class PatternTransformer {
public:
    PatternTransformer();
    
    // Set the seed notes that will be used as the basis for transformation
    void setSeedNotes(const std::vector<Note>& seeds);
    
    // Set the scale for scale-aware transformations
    void setScale(const Scale& scale);
    
    // Generate a complete pattern based on seed notes and transformation type
    Pattern generatePattern(TransformationType type, int length);
    
    // Modify an existing pattern with a transformation
    Pattern transformPattern(const Pattern& source, TransformationType type);
    
    // Preview the next few notes that would be generated
    std::vector<Note> previewTransformation(TransformationType type, int previewLength);
    
    // New methods for rhythm and articulation
    void setRhythmPattern(RhythmPattern pattern);
    void setArticulationStyle(ArticulationStyle style);
    void setGridSize(double size); // Set the basic grid size (1/4, 1/8, 1/16 note)
    
    // New pattern generation with rhythm and articulation
    Pattern generatePatternWithRhythm(TransformationType type, 
                                    RhythmPattern rhythm,
                                    ArticulationStyle articulation,
                                    int length);

    // New methods for Latin patterns
    void setTimeSignature(int numerator, int denominator);
    void setClaveDirection(bool isThreeTwo); // true for 3-2, false for 2-3
    
    // Structure to hold rhythm pattern details
    struct RhythmStep {
        double duration;
        int accent;
        bool isRest;
    };

private:
    std::vector<Note> seedNotes;
    Scale currentScale;
    std::mt19937 rng;  // Random number generator
    
    // New members for rhythm and articulation
    RhythmPattern currentRhythm;
    ArticulationStyle currentArticulation;
    double currentGridSize;
    
    // New members for Latin patterns
    bool isThreeTwoClave;
    std::vector<RhythmStep> getLatinPattern(RhythmPattern pattern);
    
    // Helper functions for different transformations
    std::vector<Note> applyStepUp(const std::vector<Note>& input);
    std::vector<Note> applyStepDown(const std::vector<Note>& input);
    std::vector<Note> applyUpTwoDownOne(const std::vector<Note>& input);
    std::vector<Note> applySkipOne(const std::vector<Note>& input);
    std::vector<Note> applyAlternateOctave(const std::vector<Note>& input);
    std::vector<Note> applyInversion(const std::vector<Note>& input);
    std::vector<Note> applyMirror(const std::vector<Note>& input);
    std::vector<Note> applyRetrograde(const std::vector<Note>& input);
    
    // New transformation helpers
    std::vector<Note> applyArch(const std::vector<Note>& input);
    std::vector<Note> applyPendulum(const std::vector<Note>& input);
    std::vector<Note> applyPowerChord(const std::vector<Note>& input);
    std::vector<Note> applyRandomFree(const std::vector<Note>& input);
    std::vector<Note> applyRandomInKey(const std::vector<Note>& input);
    std::vector<Note> applyRandomRhythmic(const std::vector<Note>& input);
    
    // New helper functions for rhythm and articulation
    double calculateNoteDuration(int position, RhythmPattern pattern);
    bool shouldBeStaccato(int position, ArticulationStyle style);
    void applySwingFeel(std::vector<Note>& notes);
    
    // New rhythm pattern implementations
    std::vector<Note> applyRhythmPattern(const std::vector<Note>& input, RhythmPattern pattern);
    std::vector<Note> applyArticulation(std::vector<Note>& notes, ArticulationStyle style);
    
    // Utility functions
    int getNextScaleNote(int currentPitch, int steps);
    int snapToScale(int pitch);
    Note createNote(int pitch, double startTime, double duration, int velocity);
    
    // Helper functions for Latin patterns
    std::vector<Note> applySambaPattern(const std::vector<Note>& input);
    std::vector<Note> applyBossaNovaPattern(const std::vector<Note>& input);
    std::vector<Note> applyRumbaPattern(const std::vector<Note>& input);
    std::vector<Note> applyMamboPattern(const std::vector<Note>& input);
    std::vector<Note> applyChaChaPattern(const std::vector<Note>& input);
    std::vector<Note> applyTangoPattern(const std::vector<Note>& input);
    std::vector<Note> applySalsaPattern(const std::vector<Note>& input);
    std::vector<Note> applyBaiaoPattern(const std::vector<Note>& input);
    std::vector<Note> applyClavePattern(const std::vector<Note>& input, bool isThreeTwo);
    std::vector<Note> applyMontuno(const std::vector<Note>& input);
    std::vector<Note> applyCascara(const std::vector<Note>& input);
    
    // Utility functions for rhythm manipulation
    std::vector<RhythmStep> createSyncopatedPattern(const std::vector<int>& accents, 
                                                   const std::vector<double>& durations);
    std::vector<Note> applyRhythmSteps(const std::vector<Note>& input,
                                      const std::vector<RhythmStep>& steps);
    std::vector<Note> applyBasicRhythmPattern(const std::vector<Note>& input, RhythmPattern pattern);
    void applyClaveAccents(std::vector<Note>& notes, bool isThreeTwo);
}; 