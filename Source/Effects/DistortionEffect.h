#pragma once
#include "../AudioEffect.h"
#include <cmath>

class DistortionEffect : public AudioEffect
{
public:
    DistortionEffect()
    {
        // Standard Tanh waveshaper
        shaper.functionToUse = [](float x) { return std::tanh(x); };
    }

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        shaper.prepare(spec);
        preGain.prepare(spec);
        postGain.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        // Drive -> Distort -> Compensate
        preGain.process(context);
        shaper.process(context);
        postGain.process(context);
    }

    void reset() override
    {
        shaper.reset();
        preGain.reset();
        postGain.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        float driveDb = 0.0f;
        if (config.hasProperty("drive") || config.getChildWithName("drive").isValid())
            driveDb = getParameterValue(config, "drive", 0.0f);
            
        preGain.setGainDecibels(driveDb);
        postGain.setGainDecibels(-driveDb * 0.5f); // Simple auto-compensation
    }

private:
    juce::dsp::WaveShaper<float> shaper;
    juce::dsp::Gain<float> preGain;
    juce::dsp::Gain<float> postGain;
};
