#pragma once
#include "../AudioEffect.h"
#include <cmath>

class DriveEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        waveShaper.prepare(spec);
        updateCurve();
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        waveShaper.process(context);
    }

    void reset() override
    {
        waveShaper.reset();
    }

    void configure(const YAML::Node& config) override
    {
        if (config["drive"]) { drive = config["drive"].as<float>(); updateCurve(); }
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("drive")) { drive = config.getProperty("drive"); updateCurve(); }
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Drive", drive, 0.0f, 1.0f, [this](float v) { drive=v; updateCurve(); } }
        };
    }

    std::string getName() const override { return "Overdrive"; }

private:
    void updateCurve()
    {
        // Simple Arctan saturation
        // drive 0..1 maps to pre-gain 1..100 or something
        float preGain = 1.0f + drive * 10.0f;

        waveShaper.functionToUse = [preGain](float x) {
            return std::atan(x * preGain) / std::atan(preGain);
        };
    }

    juce::dsp::WaveShaper<float> waveShaper;
    float drive = 0.0f;
};
