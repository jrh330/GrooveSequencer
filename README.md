# GrooveSequencer

A C++ library for musical pattern generation and transformation. This library provides tools for creating, manipulating, and transforming musical patterns with support for various rhythm styles and articulations.

## Features

- Pattern generation and transformation
- Support for multiple rhythm patterns:
  - Basic patterns (Regular, Long-Short, Short-Long, etc.)
  - Latin patterns (Samba, Bossa Nova, Rumba, etc.)
  - Clave patterns (2-3 and 3-2)
- Articulation styles (Legato, Staccato, Mixed)
- Musical transformations:
  - Step-wise movement
  - Arpeggiation
  - Inversion
  - Retrograde
  - Random patterns (Free and In-Key)
- Swing feel support
- Scale-aware transformations

## Building the Project

### Prerequisites

- CMake 3.12 or higher
- C++17 compatible compiler

### Build Instructions

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure with CMake:
```bash
cmake ..
```

3. Build:
```bash
cmake --build .
```

### Optional: Building Tests

To build with tests enabled:
```bash
cmake -DBUILD_TESTS=ON ..
cmake --build .
```

## Project Structure

- `Source/` - Source files
  - `PatternTransformer.h` - Main pattern transformation header
  - `PatternTransformer.cpp` - Implementation file
- `Tests/` - Test files (when implemented)
- `CMakeLists.txt` - CMake build configuration
- `README.md` - This file

## Usage Example

```cpp
#include "PatternTransformer.h"

// Create a transformer
PatternTransformer transformer;

// Set up a C major scale
Scale cMajor;
cMajor.root = 60; // Middle C
cMajor.intervals = {0, 2, 4, 5, 7, 9, 11}; // Major scale intervals
transformer.setScale(cMajor);

// Create seed notes
std::vector<Note> seeds = {
    {60, 100, 0.0, 0.25}, // C4
    {64, 100, 0.25, 0.25}, // E4
    {67, 100, 0.5, 0.25}, // G4
};
transformer.setSeedNotes(seeds);

// Generate a pattern
Pattern pattern = transformer.generatePatternWithRhythm(
    TransformationType::Arch,
    RhythmPattern::Samba,
    ArticulationStyle::Mixed,
    16 // length
);
```

## Future Development

- Add more rhythm patterns
- Implement pattern combination and layering
- Add MIDI file import/export
- Add real-time pattern generation
- Implement more complex transformation algorithms

## License

[Your chosen license]

## Contributing

[Your contribution guidelines] 