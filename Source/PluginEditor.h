#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ModernLookAndFeel.h"

//==============================================================================
/**
*/
class PresetEngineAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PresetEngineAudioProcessorEditor (PresetEngineAudioProcessor&);
    ~PresetEngineAudioProcessorEditor() override;

    //==============================================================================
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void rebuildUi();

    PresetEngineAudioProcessor& audioProcessor;

    ModernLookAndFeel lookAndFeel;

    juce::Label     titleLabel;
    
    // Dynamic UI
    juce::Viewport  viewport;
    std::unique_ptr<juce::Component> container;
    juce::OwnedArray<juce::Component> effectComponents;

    // Code Editor
    juce::ComboBox  languageBox;
    juce::TextButton exampleButton;
    juce::TextEditor codeEditor;
    juce::TextButton applyButton;
    juce::Label     statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetEngineAudioProcessorEditor)
};
