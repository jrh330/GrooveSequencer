#pragma once

#include <vector>

struct Note {
    int pitch;          // MIDI note number
    double startTime;   // Start time in beats
    double duration;    // Duration in beats
    int velocity;       // MIDI velocity (0-127)
    bool isRest;       // Whether this is a rest
    bool isStaccato;   // Staccato articulation flag
    int accent;        // Accent level (0 = none, 1 = medium, 2 = strong)
};

struct Pattern {
    std::vector<Note> notes;
    int length;         // Pattern length in steps
    double tempo;       // Tempo in BPM
    double gridSize;    // Grid size in beats (e.g., 0.25 for 16th notes)
}; 