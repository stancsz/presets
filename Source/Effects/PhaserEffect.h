#pragma once
#include "../AudioEffect.h"

class PhaserEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        phaser.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        phaser.process(context);
    }

    void reset() override
    {
        phaser.reset();
    }

    void configure(const YAML::Node& config) override
    {
        if (config["rate"]) { rate = config["rate"].as<float>(); phaser.setRate(rate); }
        if (config["depth"]) { depth = config["depth"].as<float>(); phaser.setDepth(depth); }
        if (config["feedback"]) { feedback = config["feedback"].as<float>(); phaser.setFeedback(feedback); }
        if (config["mix"]) { mix = config["mix"].as<float>(); phaser.setMix(mix); }
        if (config["centre_frequency"]) { centreFrequency = config["centre_frequency"].as<float>(); phaser.setCentreFrequency(centreFrequency); }
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("rate")) { rate = config.getProperty("rate"); phaser.setRate(rate); }
        if (config.hasProperty("depth")) { depth = config.getProperty("depth"); phaser.setDepth(depth); }
        if (config.hasProperty("feedback")) { feedback = config.getProperty("feedback"); phaser.setFeedback(feedback); }
        if (config.hasProperty("mix")) { mix = config.getProperty("mix"); phaser.setMix(mix); }
        if (config.hasProperty("centre_frequency")) { centreFrequency = config.getProperty("centre_frequency"); phaser.setCentreFrequency(centreFrequency); }
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Rate (Hz)", rate, 0.01f, 20.0f, [this](float v) { rate=v; phaser.setRate(v); } },
            { "Depth", depth, 0.0f, 1.0f, [this](float v) { depth=v; phaser.setDepth(v); } },
            { "Feedback", feedback, -1.0f, 1.0f, [this](float v) { feedback=v; phaser.setFeedback(v); } },
            { "Mix", mix, 0.0f, 1.0f, [this](float v) { mix=v; phaser.setMix(v); } },
            { "Centre Freq", centreFrequency, 100.0f, 5000.0f, [this](float v) { centreFrequency=v; phaser.setCentreFrequency(v); } }
        };
    }

    std::string getName() const override { return "Phaser"; }

private:
    juce::dsp::Phaser<float> phaser;
    float rate = 1.0f;
    float depth = 0.5f;
    float feedback = 0.0f;
    float mix = 0.5f;
    float centreFrequency = 1000.0f;
};
