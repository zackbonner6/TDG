cmake_minimum_required(VERSION 3.22)
project(TheDirtyGobbler VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static MSVC runtime so the .vst3 doesn't need the VC++ redistributable installed
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# --- JUCE: use a local copy if you pass -DJUCE_DIR=..., otherwise download it ---
set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")
if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(JUCE)
endif()

# Cross-compiling with MinGW can't run the VST3 manifest helper, so skip it there
set(TDG_VST3_MANIFEST TRUE)
if(CMAKE_CROSSCOMPILING)
    set(TDG_VST3_MANIFEST FALSE)
endif()

juce_add_plugin(TheDirtyGobbler
    COMPANY_NAME "Gobbler Audio"
    BUNDLE_ID com.gobbleraudio.thedirtygobbler
    PLUGIN_MANUFACTURER_CODE Gobl
    PLUGIN_CODE Tdgb
    FORMATS VST3
    PRODUCT_NAME "TheDirtyGobbler"
    DESCRIPTION "Rap vocal tuner + effects"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_AUTO_MANIFEST ${TDG_VST3_MANIFEST}
    VST3_CATEGORIES Fx Pitch Shift)

target_sources(TheDirtyGobbler PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(TheDirtyGobbler PRIVATE Source)

target_compile_definitions(TheDirtyGobbler PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(TheDirtyGobbler
    PRIVATE
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(MINGW)
    target_link_options(TheDirtyGobbler PRIVATE -static -static-libgcc -static-libstdc++)
endif()
