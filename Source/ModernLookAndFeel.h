#pragma once
#include <JuceHeader.h>

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

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width  * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Background Track (Dark ring)
        g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.strokePath(backgroundArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value Track (Cyan arc)
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Indicator (Small line or dot on the ring)
        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // Or simple dot at the end
        /*
        float dotRadius = 3.0f;
        float dotX = centreX + std::sin(angle) * radius;
        float dotY = centreY - std::cos(angle) * radius; // JUCE coordinate system might need check, but AffineTransform handles it usually.
        // Actually simple line is cleaner for "Modern" look.
        */
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
};
