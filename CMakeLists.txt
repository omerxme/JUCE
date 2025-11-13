cmake_minimum_required(VERSION 3.15)
project(APTFilter VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
)
FetchContent_MakeAvailable(JUCE)

# Plugin definition
juce_add_plugin(APTFilter
    COMPANY_NAME "Soft Analog"
    PLUGIN_MANUFACTURER_CODE Sfta
    PLUGIN_CODE Aptf
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "APT Filter"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Source files
target_sources(APTFilter PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/CustomLookAndFeel.cpp
)

# Binary resources (logo)
juce_add_binary_data(APTFilterData SOURCES Resources/logo.png)

# Link JUCE modules
target_link_libraries(APTFilter PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    APTFilterData
)

target_compile_definitions(APTFilter PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
