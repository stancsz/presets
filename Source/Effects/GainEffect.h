#pragma once
#include "../AudioEffect.h"

class GainEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        gain.prepare(spec);
        gain.setRampDurationSeconds(0.05); // Smooth parameter changes
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        gain.process(context);
    }

    void reset() override
    {
        gain.reset();
    }



    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("gain") || config.getChildWithName("gain").isValid())
            gain.setGainLinear(getParameterValue(config, "gain"));

        if (config.hasProperty("gain_db") || config.getChildWithName("gain_db").isValid())
            gain.setGainDecibels(getParameterValue(config, "gain_db"));
    }

private:
    juce::dsp::Gain<float> gain;
};
