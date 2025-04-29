#pragma once

#include <JuceHeader.h>
#include <string>

// Logging
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

// Note division enum (moved from PluginProcessor.h)
enum class NoteDivision {
    Quarter = 4,
    Eighth = 8,
    Sixteenth = 16
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
    Retrograde,
    Reverse,    // Added to match usage
    ShiftLeft,  // Added to match usage
    ShiftRight  // Added to match usage
};

// String conversion utilities
namespace EnumToString {
    inline std::string toString(LogLevel level) {
        switch (level) {
            case LogLevel::Debug: return "Debug";
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
            default: return "Unknown";
        }
    }

    inline std::string toString(NoteDivision div) {
        switch (div) {
            case NoteDivision::Quarter: return "1/4";
            case NoteDivision::Eighth: return "1/8";
            case NoteDivision::Sixteenth: return "1/16";
            default: return "Unknown";
        }
    }

    inline std::string toString(RhythmPattern pattern) {
        switch (pattern) {
            case RhythmPattern::Regular: return "Regular";
            case RhythmPattern::Dotted: return "Dotted";
            case RhythmPattern::Swing: return "Swing";
            case RhythmPattern::Syncopated: return "Syncopated";
            case RhythmPattern::Random: return "Random";
            case RhythmPattern::Clave: return "Clave";
            case RhythmPattern::LongShort: return "Long-Short";
            case RhythmPattern::ShortLong: return "Short-Long";
            case RhythmPattern::LongShortShort: return "Long-Short-Short";
            case RhythmPattern::ShortShortLong: return "Short-Short-Long";
            case RhythmPattern::DottedEighth: return "Dotted Eighth";
            case RhythmPattern::Triplet: return "Triplet";
            case RhythmPattern::Straight: return "Straight";
            case RhythmPattern::ThreeTwoClave: return "3-2 Clave";
            case RhythmPattern::TwoThreeClave: return "2-3 Clave";
            case RhythmPattern::Shuffle: return "Shuffle";
            case RhythmPattern::Custom: return "Custom";
            default: return "Unknown";
        }
    }

    inline std::string toString(ArticulationStyle style) {
        switch (style) {
            case ArticulationStyle::Legato: return "Legato";
            case ArticulationStyle::Staccato: return "Staccato";
            case ArticulationStyle::Mixed: return "Mixed";
            case ArticulationStyle::Accented: return "Accented";
            case ArticulationStyle::Random: return "Random";
            case ArticulationStyle::Pattern: return "Pattern";
            case ArticulationStyle::Normal: return "Normal";
            case ArticulationStyle::AlternatingStaccato: return "Alternating Staccato";
            case ArticulationStyle::OffbeatAccent: return "Offbeat Accent";
            case ArticulationStyle::Custom: return "Custom";
            default: return "Unknown";
        }
    }

    inline std::string toString(TransformationType type) {
        switch (type) {
            case TransformationType::StepUp: return "Step Up";
            case TransformationType::StepDown: return "Step Down";
            case TransformationType::UpTwoDownOne: return "Up 2 Down 1";
            case TransformationType::SkipOne: return "Skip One";
            case TransformationType::Arch: return "Arch";
            case TransformationType::Pendulum: return "Pendulum";
            case TransformationType::PowerChord: return "Power Chord";
            case TransformationType::RandomFree: return "Random Free";
            case TransformationType::RandomInKey: return "Random In Key";
            case TransformationType::RandomRhythmic: return "Random Rhythmic";
            case TransformationType::Invert: return "Invert";
            case TransformationType::Mirror: return "Mirror";
            case TransformationType::Retrograde: return "Retrograde";
            case TransformationType::Reverse: return "Reverse";
            case TransformationType::ShiftLeft: return "Shift Left";
            case TransformationType::ShiftRight: return "Shift Right";
            default: return "Unknown";
        }
    }
} 