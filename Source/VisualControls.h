#pragma once
#include <JuceHeader.h>
#include "EffectChain.h"

class VisualControlsComponent : public juce::Component, public EffectChain::Listener
{
public:
    VisualControlsComponent(EffectChain& chain) : effectChain(chain)
    {
        effectChain.addListener(this);
    }

    ~VisualControlsComponent() override
    {
        effectChain.removeListener(this);
    }

    void chainChanged() override
    {
        // Ensure update runs on message thread
        if (juce::MessageManager::getInstance()->isThisTheMessageThread())
            rebuild();
        else
            juce::MessageManager::callAsync([this] { rebuild(); });
    }

    void rebuild()
    {
        removeAllChildren();
        effectPanels.clear();

        // Layout: simple top-down stack of effect panels
        effectChain.visitEffects([this](AudioEffect* effect) {
            auto* panel = new EffectPanel(effect);
            effectPanels.add(panel);
            addAndMakeVisible(panel);
        });

        resized();
        repaint();
    }

    void resized() override
    {
        auto area = getLocalBounds();

        // For now, fixed height rows
        int panelHeight = 80;

        for (auto* panel : effectPanels)
        {
            panel->setBounds(area.removeFromTop(panelHeight));
            area.removeFromTop(5); // spacing
        }
    }

    void paint(juce::Graphics& g) override
    {
        if (effectPanels.isEmpty())
        {
            g.setColour(juce::Colours::grey);
            g.setFont(14.0f);
            g.drawText("No active effects or invalid config.", getLocalBounds(), juce::Justification::centred);
        }
    }

private:
    // Inner class for a single effect's controls
    class EffectPanel : public juce::Component
    {
    public:
        EffectPanel(AudioEffect* effect)
        {
            nameLabel.setText(effect->getName(), juce::dontSendNotification);
            nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
            addAndMakeVisible(nameLabel);

            auto params = effect->getParameters();
            for (const auto& p : params)
            {
                auto* slider = new juce::Slider();
                slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
                slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
                slider->setRange(p.min, p.max);
                slider->setValue(p.value, juce::dontSendNotification);

                // Capture the setter. Note: effect pointer must remain valid!
                // The Listener architecture ensures if chain is swapped, 'rebuild' is called,
                // which destroys this panel and these sliders.
                // However, there is a small race window if the chain is swapped
                // while a slider is being dragged and the message loop hasn't processed the rebuild yet.
                // In a robust app, we would hold a weak_ptr to the effect or effect handle.
                slider->onValueChange = [slider, p] {
                    p.setter((float)slider->getValue());
                };

                sliders.add(slider);
                addAndMakeVisible(slider);

                auto* label = new juce::Label();
                label->setText(p.name, juce::dontSendNotification);
                label->setJustificationType(juce::Justification::centred);
                label->setFont(12.0f);
                paramLabels.add(label);
                addAndMakeVisible(label);
            }
        }

        void paint(juce::Graphics& g) override
        {
            g.setColour(juce::Colour(0xff2d2d30)); // Slightly lighter background
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

            g.setColour(juce::Colours::black.withAlpha(0.3f));
            g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);
        }

        void resized() override
        {
            auto area = getLocalBounds().reduced(5);

            nameLabel.setBounds(area.removeFromLeft(100));

            // Distribute sliders
            int numSliders = sliders.size();
            if (numSliders > 0)
            {
                int w = area.getWidth() / numSliders;
                for (int i=0; i<numSliders; ++i)
                {
                    auto col = area.removeFromLeft(w);
                    paramLabels[i]->setBounds(col.removeFromBottom(20));
                    sliders[i]->setBounds(col);
                }
            }
        }

    private:
        juce::Label nameLabel;
        juce::OwnedArray<juce::Slider> sliders;
        juce::OwnedArray<juce::Label> paramLabels;
    };

    EffectChain& effectChain;
    juce::OwnedArray<EffectPanel> effectPanels;
};
