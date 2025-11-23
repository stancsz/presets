#pragma once
#include "../AudioEffect.h"

class ChorusEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        chorus.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        chorus.process(context);
    }

    void reset() override
    {
        chorus.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("rate") || config.getChildWithName("rate").isValid())
            chorus.setRate(getParameterValue(config, "rate", 1.0f));
            
        if (config.hasProperty("depth") || config.getChildWithName("depth").isValid())
            chorus.setDepth(getParameterValue(config, "depth", 0.25f));
            
        if (config.hasProperty("delay") || config.getChildWithName("delay").isValid())
            chorus.setCentreDelay(getParameterValue(config, "delay", 7.0f));
            
        if (config.hasProperty("feedback") || config.getChildWithName("feedback").isValid())
            chorus.setFeedback(getParameterValue(config, "feedback", 0.0f));
            
        if (config.hasProperty("mix") || config.getChildWithName("mix").isValid())
            chorus.setMix(getParameterValue(config, "mix", 0.5f));
    }

private:
    juce::dsp::Chorus<float> chorus;
};
