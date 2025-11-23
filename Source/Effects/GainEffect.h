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

    void configure(const YAML::Node& config) override
    {
        if (config["gain"])
            setGain(config["gain"].as<float>());

        if (config["gain_db"])
            gain.setGainDecibels(config["gain_db"].as<float>());
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("gain"))
            setGain(config.getProperty("gain"));

        if (config.hasProperty("gain_db"))
            gain.setGainDecibels(config.getProperty("gain_db"));
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Gain", currentGain, 0.0f, 2.0f, [this](float v) { setGain(v); } }
        };
    }

    std::string getName() const override { return "Gain"; }

private:
    void setGain(float g)
    {
        currentGain = g;
        gain.setGainLinear(currentGain);
    }

    juce::dsp::Gain<float> gain;
    float currentGain = 1.0f;
};
