#pragma once
#include <JuceHeader.h>
#include <cmath>

class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel()
    {
        // Dark theme colors - Modern Material Dark
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff121212)); // Very dark background
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e1e1e));      // Slightly lighter for input
        setColour(juce::TextEditor::textColourId, juce::Colour(0xffe0e0e0));            // Off-white text
        setColour(juce::TextEditor::highlightColourId, juce::Colour(0xff264f78));
        setColour(juce::CaretComponent::caretColourId, juce::Colours::cyan);

        // Buttons
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00bcd4)); // Cyan for active
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffb0b0b0));
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        // Labels
        setColour(juce::Label::textColourId, juce::Colour(0xffb0b0b0));

        // Sliders
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00bcd4)); // Cyan
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2d2d2d));
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffe0e0e0));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff2d2d2d));
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1e1e1e));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = 4.0f;
        auto bounds = button.getLocalBounds().toFloat();

        auto baseColour = backgroundColour;
        if (shouldDrawButtonAsDown)
            baseColour = baseColour.darker(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.1f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);

        if (button.hasKeyboardFocus(true))
        {
            g.setColour(juce::Colours::cyan.withAlpha(0.4f));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider&) override
    {
        // 1. Calculate geometry
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto center = juce::Point<float> ((float) x + (float) width * 0.5f, (float) y + (float) height * 0.5f);
        auto rx = center.x - radius;
        auto ry = center.y - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // 2. Draw Background (Dark Track)
        g.setColour (juce::Colours::darkgrey);
        g.drawEllipse (rx, ry, rw, rw, 2.0f);

        // 3. Draw Active Arc (Cyan Progress)
        juce::Path p;
        p.addCentredArc (center.x, center.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (juce::Colour::fromString("FF00E5FF")); // Cyan
        g.strokePath (p, juce::PathStrokeType (2.0f));

        // 4. Draw Needle/Indicator (Visual Feedback)
        juce::Path needle;
        needle.addRectangle(-2.0f, -radius, 4.0f, 10.0f);
        g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(center));
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearVertical)
        {
            auto trackWidth = 4.0f;
            auto trackX = (float)x + (float)width * 0.5f - trackWidth * 0.5f;
            auto trackH = (float)height;

            // Background Track
            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.fillRoundedRectangle(trackX, (float)y, trackWidth, trackH, 2.0f);

            // Value Track (Fill from bottom)
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId)); // Reuse cyan
            auto fillH = (float)height - sliderPos; // sliderPos is the pixel position of the thumb
            g.fillRoundedRectangle(trackX, sliderPos, trackWidth, (float)y + (float)height - sliderPos, 2.0f);

            // Thumb (Simple Horizontal Bar or Circle)
            auto thumbH = 8.0f;
            auto thumbW = 16.0f;
            g.setColour(juce::Colours::white);
            g.fillRoundedRectangle((float)x + (float)width * 0.5f - thumbW * 0.5f, sliderPos - thumbH * 0.5f, thumbW, thumbH, 2.0f);
        }
        else // Horizontal
        {
             // Similar logic for horizontal...
            auto trackHeight = 4.0f;
            auto trackY = (float)y + (float)height * 0.5f - trackHeight * 0.5f;

            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.fillRoundedRectangle((float)x, trackY, (float)width, trackHeight, 2.0f);

            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.fillRoundedRectangle((float)x, trackY, sliderPos - (float)x, trackHeight, 2.0f);

            auto thumbW = 8.0f;
            auto thumbH = 16.0f;
            g.setColour(juce::Colours::white);
            g.fillRoundedRectangle(sliderPos - thumbW * 0.5f, (float)y + (float)height * 0.5f - thumbH * 0.5f, thumbW, thumbH, 2.0f);
        }
    }
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override
    {
        auto cornerSize = 4.0f;
        juce::Rectangle<float> boxBounds (0.0f, 0.0f, (float) width, (float) height);

        // 1. Flat Dark Background
        g.setColour (juce::Colour::fromString("FF1E1E1E"));
        g.fillRoundedRectangle (boxBounds, cornerSize);

        // 2. Minimal Border
        g.setColour (juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle (boxBounds, cornerSize, 1.0f);

        // 3. Custom Arrow (Chevron)
        juce::Path arrow;
        auto arrowSize = 5.0f;
        auto arrowCenter = juce::Point<float> ((float) (width - 15), height * 0.5f);
        
        // Simple V shape
        arrow.startNewSubPath (arrowCenter.x - arrowSize, arrowCenter.y - arrowSize * 0.5f);
        arrow.lineTo (arrowCenter.x, arrowCenter.y + arrowSize * 0.5f);
        arrow.lineTo (arrowCenter.x + arrowSize, arrowCenter.y - arrowSize * 0.5f);

        g.setColour (juce::Colours::lightgrey);
        g.strokePath (arrow, juce::PathStrokeType (1.5f));
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            const bool isSeparator, const bool isActive,
                            const bool isHighlighted, const bool isTicked,
                            const bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        if (isSeparator)
        {
            g.setColour (juce::Colours::white.withAlpha (0.1f));
            g.fillRect (area.reduced (5, 0).removeFromTop (1));
            return;
        }

        auto r = area.toFloat();
        
        // 1. Background Coloring
        if (isHighlighted)
        {
            // Hover state
            g.setColour (juce::Colour::fromString("FF252525")); 
            g.fillRect (r);
        }
        else if (isTicked) 
        {
            // Selected state (Active Item) - Subtle distinction
            g.setColour (juce::Colour::fromString("FF1E1E1E")); 
            g.fillRect (r);
        }
        else 
        {
            // Default Background
            g.setColour (juce::Colour::fromString("FF121212"));
            g.fillRect (r);
        }

        // 2. Selection Indicator (The "Designer" replacement for the checkmark)
        if (isTicked)
        {
            g.setColour (juce::Colour::fromString("FF00E5FF")); // Cyan Accent
            // Draw a thin vertical strip on the left instead of a checkmark
            g.fillRect (r.removeFromLeft(4.0f)); 
        }

        // 3. Text
        g.setColour (isHighlighted ? juce::Colours::white : juce::Colours::lightgrey);
        g.setFont (getPopupMenuFont());
        
        // Left-align text with padding to account for the accent strip
        g.drawText (text, r.reduced (10, 0), juce::Justification::centredLeft, true);
    }
};
