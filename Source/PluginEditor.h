#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ModernLookAndFeel.h"

//==============================================================================
/**
 * A dummy spectrum visualizer component that draws a pleasing gradient curve.
 * In a real plugin, this would read FFT data from the processor.
 */
class SpectrumComponent : public juce::Component, public juce::Timer
{
public:
    SpectrumComponent()
    {
        startTimerHz(30); // Animate at 30fps
    }

    ~SpectrumComponent() override
    {
        stopTimer();
    }

    void timerCallback() override
    {
        phase += 0.05f;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xff121212));
        g.fillRect(area);

        // Grid (Subtle)
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        for (float x = 0; x < area.getWidth(); x += 40.0f)
            g.drawVerticalLine((int)x, 0.0f, area.getHeight());
        for (float y = 0; y < area.getHeight(); y += 40.0f)
            g.drawHorizontalLine((int)y, 0.0f, area.getWidth());

        // Draw Spectrum Curve
        juce::Path path;
        path.startNewSubPath(0, area.getHeight());

        int numPoints = 100;
        float w = area.getWidth();
        float h = area.getHeight();

        for (int i = 0; i <= numPoints; ++i)
        {
            float x = (float)i / numPoints;

            // Sum of sines to simulate audio spectrum
            float yNorm = 0.0f;
            yNorm += 0.3f * std::sin(x * 10.0f + phase);
            yNorm += 0.2f * std::sin(x * 23.0f - phase * 1.5f);
            yNorm += 0.1f * std::sin(x * 50.0f + phase * 2.0f);

            // Shape it (lows higher, highs lower usually)
            float envelope = std::exp(-2.0f * x);
            yNorm *= envelope;

            // Map to height
            float y = h - (yNorm + 0.1f) * h * 0.8f;
            y = juce::jlimit(0.0f, h, y);

            path.lineTo(x * w, y);
        }

        path.lineTo(w, h);
        path.closeSubPath();

        // Gradient Fill
        juce::ColourGradient gradient(
            juce::Colour(0xff00bcd4), 0, h * 0.5f,
            juce::Colour(0xff00bcd4).withAlpha(0.0f), 0, h,
            false);
        g.setGradientFill(gradient);
        g.fillPath(path);

        // Stroke
        g.setColour(juce::Colour(0xff00bcd4));
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

private:
    float phase = 0.0f;
};

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
    
    // Left Column Components
    SpectrumComponent spectrumComponent;

    juce::ComboBox  languageBox;
    juce::TextButton exampleButton;
    juce::TextEditor codeEditor;
    juce::TextButton applyButton;
    juce::Label     statusLabel;

    // Right Column Components
    juce::Viewport  viewport;
    std::unique_ptr<juce::Component> container;
    juce::OwnedArray<juce::Component> effectComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetEngineAudioProcessorEditor)
};
