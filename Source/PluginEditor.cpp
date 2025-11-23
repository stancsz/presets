#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
YamlPresetPluginAudioProcessorEditor::YamlPresetPluginAudioProcessorEditor (YamlPresetPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), visualControls(p.getChain())
{
    // Apply custom look and feel
    setLookAndFeel(&lookAndFeel);

    setSize (600, 500);

    // Header
    titleLabel.setText("Yaml Preset Plugin", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // View Toggle
    toggleViewButton.setButtonText("Switch to Visual View");
    toggleViewButton.onClick = [this] {
        isVisualMode = !isVisualMode;
        toggleViewButton.setButtonText(isVisualMode ? "Switch to Code View" : "Switch to Visual View");
        updateView();
    };
    addAndMakeVisible(toggleViewButton);

    // Code Editor
    codeEditor.setMultiLine(true);
    codeEditor.setReturnKeyStartsNewLine(true);
    codeEditor.setTabKeyUsedAsCharacter(true);
    codeEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    codeEditor.setText(audioProcessor.getCurrentConfig());
    codeEditor.setLeftIndent(10);
    addChildComponent(codeEditor); // Visible by default via updateView

    // Visual Controls
    addChildComponent(visualControls);

    // Status
    statusLabel.setText("Ready.", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(statusLabel);

    // Apply Button
    applyButton.setButtonText("Apply Configuration");
    applyButton.onClick = [this] {
        auto result = audioProcessor.loadConfig(codeEditor.getText());
        if (result.wasOk())
        {
            statusLabel.setText("Configuration Loaded Successfully.", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);

            // If we are in visual mode, or even if not, we should notify controls to update if they are visible
            // But logic is: Apply is only visible in Code mode.
            // If user switches to Visual mode, we rebuild.
        }
        else
        {
            statusLabel.setText(result.getErrorMessage(), juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        }
    };
    addChildComponent(applyButton);

    updateView();
}

YamlPresetPluginAudioProcessorEditor::~YamlPresetPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void YamlPresetPluginAudioProcessorEditor::updateView()
{
    codeEditor.setVisible(!isVisualMode);
    applyButton.setVisible(!isVisualMode);
    visualControls.setVisible(isVisualMode);

    if (isVisualMode)
    {
        visualControls.rebuild();
    }

    resized();
}

//==============================================================================
void YamlPresetPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void YamlPresetPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Header Area
    auto headerArea = area.removeFromTop(40);
    titleLabel.setBounds(headerArea.removeFromLeft(200));
    toggleViewButton.setBounds(headerArea.removeFromRight(150));

    // Footer (Status)
    auto footerArea = area.removeFromBottom(40);
    statusLabel.setBounds(footerArea.removeFromLeft(footerArea.getWidth() / 2));

    if (applyButton.isVisible())
        applyButton.setBounds(footerArea.removeFromRight(150));

    // Spacing
    area.removeFromTop(10);
    area.removeFromBottom(10);

    // Content
    if (codeEditor.isVisible())
        codeEditor.setBounds(area);

    if (visualControls.isVisible())
        visualControls.setBounds(area);
}
