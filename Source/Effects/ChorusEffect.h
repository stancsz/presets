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

    void configure(const YAML::Node& config) override
    {
        if (config["rate"]) { rate = config["rate"].as<float>(); chorus.setRate(rate); }
        if (config["depth"]) { depth = config["depth"].as<float>(); chorus.setDepth(depth); }
        if (config["feedback"]) { feedback = config["feedback"].as<float>(); chorus.setFeedback(feedback); }
        if (config["mix"]) { mix = config["mix"].as<float>(); chorus.setMix(mix); }
        if (config["delay"]) { centreDelay = config["delay"].as<float>(); chorus.setCentreDelay(centreDelay); }
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("rate")) { rate = config.getProperty("rate"); chorus.setRate(rate); }
        if (config.hasProperty("depth")) { depth = config.getProperty("depth"); chorus.setDepth(depth); }
        if (config.hasProperty("feedback")) { feedback = config.getProperty("feedback"); chorus.setFeedback(feedback); }
        if (config.hasProperty("mix")) { mix = config.getProperty("mix"); chorus.setMix(mix); }
        if (config.hasProperty("delay")) { centreDelay = config.getProperty("delay"); chorus.setCentreDelay(centreDelay); }
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Rate (Hz)", rate, 0.01f, 20.0f, [this](float v) { rate=v; chorus.setRate(v); } },
            { "Depth", depth, 0.0f, 1.0f, [this](float v) { depth=v; chorus.setDepth(v); } },
            { "Feedback", feedback, -1.0f, 1.0f, [this](float v) { feedback=v; chorus.setFeedback(v); } },
            { "Mix", mix, 0.0f, 1.0f, [this](float v) { mix=v; chorus.setMix(v); } },
            { "Centre Delay (ms)", centreDelay, 1.0f, 100.0f, [this](float v) { centreDelay=v; chorus.setCentreDelay(v); } }
        };
    }

    std::string getName() const override { return "Chorus"; }

private:
    juce::dsp::Chorus<float> chorus;
    float rate = 1.0f;
    float depth = 0.25f;
    float feedback = 0.0f;
    float mix = 0.5f;
    float centreDelay = 7.0f;
};
