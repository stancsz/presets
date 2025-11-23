#pragma once
#include "../AudioEffect.h"

class LimiterEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        limiter.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        limiter.process(context);
    }

    void reset() override
    {
        limiter.reset();
    }



    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("threshold")) limiter.setThreshold(config.getProperty("threshold"));
        if (config.hasProperty("release")) limiter.setRelease(config.getProperty("release"));
    }

private:
    juce::dsp::Limiter<float> limiter;
};
