#pragma once
#include "../AudioEffect.h"

class CompressorEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        compressor.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        compressor.process(context);
    }

    void reset() override
    {
        compressor.reset();
    }

    void configure(const YAML::Node& config) override
    {
        if (config["threshold"]) { threshold = config["threshold"].as<float>(); compressor.setThreshold(threshold); }
        if (config["ratio"]) { ratio = config["ratio"].as<float>(); compressor.setRatio(ratio); }
        if (config["attack"]) { attack = config["attack"].as<float>(); compressor.setAttack(attack); }
        if (config["release"]) { release = config["release"].as<float>(); compressor.setRelease(release); }
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("threshold")) { threshold = config.getProperty("threshold"); compressor.setThreshold(threshold); }
        if (config.hasProperty("ratio")) { ratio = config.getProperty("ratio"); compressor.setRatio(ratio); }
        if (config.hasProperty("attack")) { attack = config.getProperty("attack"); compressor.setAttack(attack); }
        if (config.hasProperty("release")) { release = config.getProperty("release"); compressor.setRelease(release); }
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Threshold", threshold, -60.0f, 0.0f, [this](float v) { threshold=v; compressor.setThreshold(v); } },
            { "Ratio", ratio, 1.0f, 20.0f, [this](float v) { ratio=v; compressor.setRatio(v); } },
            { "Attack", attack, 0.0f, 100.0f, [this](float v) { attack=v; compressor.setAttack(v); } },
            { "Release", release, 0.0f, 500.0f, [this](float v) { release=v; compressor.setRelease(v); } }
        };
    }

    std::string getName() const override { return "Compressor"; }

private:
    juce::dsp::Compressor<float> compressor;
    float threshold = 0.0f;
    float ratio = 1.0f;
    float attack = 30.0f;
    float release = 100.0f;
};
