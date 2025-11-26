cmake_minimum_required(VERSION 3.15)

project(PresetEngine VERSION 0.1.0)

include(FetchContent)

# --- JUCE ---
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
)

# --- yaml-cpp ---
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
)

FetchContent_MakeAvailable(JUCE yaml-cpp)

# --- Plugin ---
# Note: Building this requires X11 development headers (e.g. libxrandr-dev, libxinerama-dev, etc.) on Linux.

juce_add_plugin(PresetEngine
    COMPANY_NAME "Stancsz Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE NAPT
    PLUGIN_CODE PrEn
    FORMATS VST3 Standalone
    PRODUCT_NAME "Preset Engine"
)

target_sources(PresetEngine
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/AudioEffect.h
        Source/EffectChain.h
        Source/EffectChain.cpp
        Source/Effects/GainEffect.h
        Source/Effects/FilterEffect.h
        Source/Effects/CompressorEffect.h
        Source/Effects/LimiterEffect.h
        Source/Effects/ReverbEffect.h
        Source/Effects/ChorusEffect.h
        Source/Effects/DistortionEffect.h
        Source/Effects/PhaserEffect.h
        Source/Effects/DelayEffect.h
        Source/Effects/NoiseGateEffect.h
        Source/Effects/LadderFilterEffect.h
        Source/Effects/PannerEffect.h
)

target_compile_definitions(PresetEngine PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PresetEngine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
        yaml-cpp
)

target_compile_features(PresetEngine PUBLIC cxx_std_17)

juce_generate_juce_header(PresetEngine)

# --- Screenshot Test ---
juce_add_gui_app(ScreenshotTest
    PRODUCT_NAME "ScreenshotTest"
    VERSION 1.0.0
)

target_sources(ScreenshotTest
    PRIVATE
        Tests/ScreenshotTest.cpp
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/AudioEffect.h
        Source/EffectChain.h
        Source/EffectChain.cpp
        Source/Effects/GainEffect.h
        Source/Effects/FilterEffect.h
        Source/Effects/CompressorEffect.h
        Source/Effects/LimiterEffect.h
        Source/Effects/ReverbEffect.h
        Source/Effects/ChorusEffect.h
        Source/Effects/DistortionEffect.h
        Source/Effects/PhaserEffect.h
        Source/Effects/DelayEffect.h
        Source/Effects/NoiseGateEffect.h
        Source/Effects/LadderFilterEffect.h
        Source/Effects/PannerEffect.h
        Source/ModernLookAndFeel.h
)

target_include_directories(ScreenshotTest PRIVATE Source)

target_link_libraries(ScreenshotTest
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
        yaml-cpp
)

target_compile_definitions(ScreenshotTest PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    JucePlugin_Name="Preset Engine"
    JucePlugin_VersionString="0.1.0"
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_IsSynth=0
    JucePlugin_EditorRequiresKeyboardFocus=1
)

juce_generate_juce_header(ScreenshotTest)
