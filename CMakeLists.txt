cmake_minimum_required(VERSION 3.15)

project(YamlPresetPlugin VERSION 0.0.1)

include(FetchContent)

# --- JUCE ---
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
)

# --- yaml-cpp ---
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
)

FetchContent_MakeAvailable(JUCE yaml-cpp)

# --- Plugin ---
# Note: Building this requires X11 development headers (e.g. libxrandr-dev, libxinerama-dev, etc.) on Linux.

juce_add_plugin(YamlPresetPlugin
    COMPANY_NAME "MyCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE Yaml
    FORMATS VST3 Standalone
    PRODUCT_NAME "YamlPresetPlugin"
)

target_sources(YamlPresetPlugin
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

target_compile_definitions(YamlPresetPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(YamlPresetPlugin
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

target_compile_features(YamlPresetPlugin PUBLIC cxx_std_17)

juce_generate_juce_header(YamlPresetPlugin)
