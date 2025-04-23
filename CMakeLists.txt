cmake_minimum_required(VERSION 3.15)
project(GrooveSequencer VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Initialize JUCE
juce_add_plugin(GrooveSequencer
    VERSION 1.0.0                            # Set this if you want to update the version number
    COMPANY_NAME "YourCompany"               # Replace with your company name
    BUNDLE_ID "com.yourcompany.groovesequencer"  # Fixed bundle ID without spaces
    IS_SYNTH TRUE                           # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE                   # Does it need midi input?
    NEEDS_MIDI_OUTPUT TRUE                  # Does it produce midi output?
    IS_MIDI_EFFECT TRUE                     # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE       # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE            # Should the plugin be installed after building?
    PLUGIN_MANUFACTURER_CODE Juce           # A four-character manufacturer id
    PLUGIN_CODE Gseq                        # A unique four-character plugin id
    FORMATS AU VST3                         # The formats to build. Other valid formats: VST AAX Unity
    PRODUCT_NAME "GrooveSequencer"          # The name of the final executable
)

# Add source files
target_sources(GrooveSequencer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/PatternTransformer.cpp
)

# Add compiler warnings using standard target_compile_options
target_compile_options(GrooveSequencer
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Link with JUCE modules
target_link_libraries(GrooveSequencer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_basics
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Add tests if enabled
option(BUILD_TESTS "Build test executable" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif() 