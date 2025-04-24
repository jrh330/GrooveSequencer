cmake_minimum_required(VERSION 3.15)
project(GrooveSequencer VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE if not already available
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.5
)
FetchContent_MakeAvailable(juce)

# Add JUCE modules
juce_add_plugin(GrooveSequencer
    VERSION 1.0.0
    COMPANY_NAME "Your Company"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Ycom
    PLUGIN_CODE Grsq
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Groove Sequencer"
)

# Generate JuceHeader.h
juce_generate_juce_header(GrooveSequencer)

# Add source files
target_sources(GrooveSequencer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/PatternTransformer.cpp
        Source/GrooveSequencerLookAndFeel.cpp
        Source/Components/GridSequencerComponent.cpp
        Source/Components/PatternBrowserComponent.cpp
        Source/Components/PatternControlsComponent.cpp
        Source/Components/TransportComponent.cpp
)

# Set include directories
target_include_directories(GrooveSequencer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Link JUCE modules
target_compile_definitions(GrooveSequencer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(GrooveSequencer
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
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