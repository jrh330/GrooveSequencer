cmake_minimum_required(VERSION 3.12)
project(GrooveSequencer VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_library(GrooveSequencer
    Source/PatternTransformer.cpp
)

# Include directories
target_include_directories(GrooveSequencer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Add compiler warnings
if(MSVC)
    target_compile_options(GrooveSequencer PRIVATE /W4)
else()
    target_compile_options(GrooveSequencer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Create test executable
option(BUILD_TESTS "Build test executable" OFF)
if(BUILD_TESTS)
    add_executable(GrooveSequencerTests
        Tests/main.cpp
    )
    target_link_libraries(GrooveSequencerTests PRIVATE GrooveSequencer)
endif() 