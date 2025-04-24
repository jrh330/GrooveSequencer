# GrooveSequencer

A modern MIDI groove sequencer plugin with an Olivetti-inspired modernist interface. Built with JUCE framework.

## Features

- 16x4 grid sequencer with velocity and accent control
- Modernist UI design inspired by Olivetti aesthetics
- Multiple articulation modes (Legato, Staccato, Mixed, Pattern)
- Flexible grid size options (1/4, 1/8, 1/16, 1/32 with triplet and dotted variations)
- Swing control
- Velocity scaling
- Gate length control
- DAW sync support
- Pattern transformation capabilities

## Building

### Prerequisites

- JUCE 7.0.0 or later
- C++17 compatible compiler
- CMake 3.15 or later

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/yourusername/GrooveSequencer.git
cd GrooveSequencer
```

2. Generate project files using CMake:
```bash
cmake -B Builds
```

3. Build the project:
```bash
cmake --build Builds
```

## Usage

The plugin can be loaded in any DAW that supports VST3, AU, or AAX formats. The main interface consists of:

- Transport controls (Play/Stop, Tempo, DAW Sync)
- Grid size selector with triplet and dotted note options
- Articulation selector
- Grid controls (Swing, Velocity Scale, Gate Length)
- 16x4 sequencer grid with velocity and accent control

### Grid Interaction

- Left click: Toggle step on/off
- Vertical drag: Adjust velocity
- Right click: Cycle through accent levels
- Alt + click: Toggle staccato

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- UI design inspired by Olivetti's modernist aesthetic
- Built with the JUCE framework 