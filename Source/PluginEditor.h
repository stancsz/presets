#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ModernLookAndFeel.h"
#include "VisualControls.h"

//==============================================================================
/**
*/
class YamlPresetPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    YamlPresetPluginAudioProcessorEditor (YamlPresetPluginAudioProcessor&);
    ~YamlPresetPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void updateView();

    YamlPresetPluginAudioProcessor& audioProcessor;

    ModernLookAndFeel lookAndFeel;

    juce::Label     titleLabel;

    // Mode Toggle
    juce::TextButton toggleViewButton;
    bool isVisualMode = false;

    // Code View
    juce::TextEditor codeEditor;
    juce::TextButton applyButton;

    // Visual View
    VisualControlsComponent visualControls;

    juce::Label     statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YamlPresetPluginAudioProcessorEditor)
};
