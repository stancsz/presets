#pragma once
#include "../AudioEffect.h"

class PannerEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        panner.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        panner.process(context);
    }

    void reset() override
    {
        panner.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("pan") || config.getChildWithName("pan").isValid())
            panner.setPan(getParameterValue(config, "pan", 0.0f));
            
        if (config.hasProperty("rule"))
        {
            juce::String rule = config.getProperty("rule").toString();
            if (rule.equalsIgnoreCase("linear")) panner.setRule(juce::dsp::PannerRule::linear);
            else if (rule.equalsIgnoreCase("balanced")) panner.setRule(juce::dsp::PannerRule::balanced);
            else if (rule.equalsIgnoreCase("sin3db")) panner.setRule(juce::dsp::PannerRule::sin3dB);
            else if (rule.equalsIgnoreCase("sin4.5db")) panner.setRule(juce::dsp::PannerRule::sin4p5dB);
            else if (rule.equalsIgnoreCase("sin6db")) panner.setRule(juce::dsp::PannerRule::sin6dB);
            else if (rule.equalsIgnoreCase("square")) panner.setRule(juce::dsp::PannerRule::squareRoot3dB);
        }
    }

private:
    juce::dsp::Panner<float> panner;
};
