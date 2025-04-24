#pragma once

#include <JuceHeader.h>

// Logging
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

// Rhythm patterns
enum class RhythmPattern {
    Regular,
    Dotted,
    Swing,
    Syncopated,
    Random,
    Clave,
    LongShort,
    ShortLong,
    LongShortShort,
    ShortShortLong,
    DottedEighth,
    Triplet,
    Straight,
    ThreeTwoClave,
    TwoThreeClave,
    Shuffle,
    Custom
};

// Articulation styles
enum class ArticulationStyle {
    Legato,
    Staccato,
    Mixed,
    Accented,
    Random,
    Pattern,
    Normal,
    AlternatingStaccato,
    OffbeatAccent,
    Custom
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