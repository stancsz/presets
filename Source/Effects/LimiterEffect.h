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

    void configure(const YAML::Node& config) override
    {
        if (config["threshold"]) { threshold = config["threshold"].as<float>(); limiter.setThreshold(threshold); }
        if (config["release"]) { release = config["release"].as<float>(); limiter.setRelease(release); }
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("threshold")) { threshold = config.getProperty("threshold"); limiter.setThreshold(threshold); }
        if (config.hasProperty("release")) { release = config.getProperty("release"); limiter.setRelease(release); }
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Threshold", threshold, -60.0f, 0.0f, [this](float v) { threshold=v; limiter.setThreshold(v); } },
            { "Release", release, 0.0f, 500.0f, [this](float v) { release=v; limiter.setRelease(v); } }
        };
    }

    std::string getName() const override { return "Limiter"; }

private:
    juce::dsp::Limiter<float> limiter;
    float threshold = -0.1f;
    float release = 10.0f;
};
