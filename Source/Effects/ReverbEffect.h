#pragma once
#include "../AudioEffect.h"

class ReverbEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        reverb.prepare(spec);
        updateParams();
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        reverb.process(context);
    }

    void reset() override
    {
        reverb.reset();
    }

    void configure(const YAML::Node& config) override
    {
        if (config["room_size"]) roomSize = config["room_size"].as<float>();
        if (config["damping"]) damping = config["damping"].as<float>();
        if (config["wet"]) wetLevel = config["wet"].as<float>();
        if (config["dry"]) dryLevel = config["dry"].as<float>();
        if (config["width"]) width = config["width"].as<float>();
        updateParams();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("room_size")) roomSize = config.getProperty("room_size");
        if (config.hasProperty("damping")) damping = config.getProperty("damping");
        if (config.hasProperty("wet")) wetLevel = config.getProperty("wet");
        if (config.hasProperty("dry")) dryLevel = config.getProperty("dry");
        if (config.hasProperty("width")) width = config.getProperty("width");
        updateParams();
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Room Size", roomSize, 0.0f, 1.0f, [this](float v) { roomSize=v; updateParams(); } },
            { "Damping", damping, 0.0f, 1.0f, [this](float v) { damping=v; updateParams(); } },
            { "Wet Level", wetLevel, 0.0f, 1.0f, [this](float v) { wetLevel=v; updateParams(); } },
            { "Dry Level", dryLevel, 0.0f, 1.0f, [this](float v) { dryLevel=v; updateParams(); } },
            { "Width", width, 0.0f, 1.0f, [this](float v) { width=v; updateParams(); } }
        };
    }

    std::string getName() const override { return "Reverb"; }

private:
    void updateParams()
    {
        juce::dsp::Reverb::Parameters params;
        params.roomSize = roomSize;
        params.damping = damping;
        params.wetLevel = wetLevel;
        params.dryLevel = dryLevel;
        params.width = width;
        params.freezeMode = 0;
        reverb.setParameters(params);
    }

    juce::dsp::Reverb reverb;
    float roomSize = 0.5f;
    float damping = 0.5f;
    float wetLevel = 0.33f;
    float dryLevel = 0.4f;
    float width = 1.0f;
};
