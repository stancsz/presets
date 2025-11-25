#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ModernLookAndFeel.h"
#include <cmath>

//==============================================================================
/**
 * A dummy spectrum visualizer component that draws a pleasing gradient curve.
 * In a real plugin, this would read FFT data from the processor.
 */
class SpectrumComponent : public juce::Component, public juce::Timer
{
public:
    SpectrumComponent(PresetEngineAudioProcessor& p) : processor(p)
    {
        startTimerHz(60); // Smoother animation
    }

    ~SpectrumComponent() override
    {
        stopTimer();
    }

    void timerCallback() override
    {
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        float w = area.getWidth();
        float h = area.getHeight();
        float midY = h * 0.5f;

        // Background
        g.setColour(juce::Colour(0xff121212));
        g.fillRect(area);

        // Grid (Subtle)
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawHorizontalLine((int)midY, 0.0f, w);

        // Helper to draw waveform
        auto drawWaveform = [&](const std::vector<float>& data, int writePos, juce::Colour color, float alpha)
        {
            if (data.empty()) return;

            g.setColour(color.withAlpha(alpha));
            juce::Path path;
            
            // Simple trigger logic: find first zero crossing
            int startIdx = 0;
            int size = (int)data.size();
            
            // Try to find a stable trigger point in the last N samples
            // This is a naive trigger but better than rolling
            // Actually, just drawing the buffer as a circular buffer is easiest for now
            // To make it look like a "window", we start from writePos (oldest sample)
            
            path.startNewSubPath(0, midY);
            
            int readPos = writePos;
            
            // Draw 512 samples or so
            int numToDraw = std::min(size, 512);
            float xInc = w / (float)numToDraw;
            
            for (int i = 0; i < numToDraw; ++i)
            {
                float sample = data[readPos];
                float y = midY - (sample * h * 0.4f); // Scale amplitude
                
                if (i == 0) path.startNewSubPath(0, y);
                else path.lineTo((float)i * xInc, y);
                
                readPos = (readPos + 1) % size;
            }
            
            g.strokePath(path, juce::PathStrokeType(1.5f));
        };

        // Draw Input (Grey)
        drawWaveform(processor.inputVisuals.data, processor.inputVisuals.writePos.load(), juce::Colours::grey, 0.5f);

        // Draw Output (Blue)
        drawWaveform(processor.outputVisuals.data, processor.outputVisuals.writePos.load(), juce::Colour(0xff00bcd4), 0.9f);
    }

private:
    PresetEngineAudioProcessor& processor;
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
    juce::String parsePythonToYaml(const juce::String& pythonCode);

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
