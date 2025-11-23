#pragma once
#include "../AudioEffect.h"

class NoiseGateEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        gate.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        gate.process(context);
    }

    void reset() override
    {
        gate.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("threshold") || config.getChildWithName("threshold").isValid())
            gate.setThreshold(getParameterValue(config, "threshold", -60.0f));
            
        if (config.hasProperty("ratio") || config.getChildWithName("ratio").isValid())
            gate.setRatio(getParameterValue(config, "ratio", 2.0f));
            
        if (config.hasProperty("attack") || config.getChildWithName("attack").isValid())
            gate.setAttack(getParameterValue(config, "attack", 2.0f));
            
        if (config.hasProperty("release") || config.getChildWithName("release").isValid())
            gate.setRelease(getParameterValue(config, "release", 100.0f));
    }

private:
    juce::dsp::NoiseGate<float> gate;
};
