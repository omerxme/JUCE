cmake_minimum_required(VERSION 3.15)
project(APTWide VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(APTWide
    COMPANY_NAME "Soft Analog"
    PLUGIN_MANUFACTURER_CODE Sfta
    PLUGIN_CODE Aptw
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "APT Wide"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(APTWide PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    CustomLookAndFeel.cpp
    PresetManager.cpp
)

juce_add_binary_data(APTWideData SOURCES ../Resources/logo.png)

target_link_libraries(APTWide PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    APTWideData
)

target_compile_definitions(APTWide PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
