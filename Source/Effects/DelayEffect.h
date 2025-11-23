#pragma once
#include "../AudioEffect.h"

class DelayEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        delayLine.prepare(spec);
        delayLine.setMaximumDelayInSamples(static_cast<int>(2.0 * sampleRate)); // Max 2 seconds
        updateDelay();
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        // Simple mix implementation: Dry/Wet
        // For simplicity in this wrapper, we'll just process wet for now or implement a manual mix
        // standard juce::dsp::DelayLine is just the line.

        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        for (size_t ch = 0; ch < inputBlock.getNumChannels(); ++ch)
        {
            auto* in = inputBlock.getChannelPointer(ch);
            auto* out = outputBlock.getChannelPointer(ch);
            auto numSamples = inputBlock.getNumSamples();

            for (size_t i = 0; i < numSamples; ++i)
            {
                delayLine.pushSample((int)ch, in[i] + lastOutput[ch] * feedback);
                float wetSignal = delayLine.popSample((int)ch);

                lastOutput[ch] = wetSignal;
                out[i] = in[i] * (1.0f - mix) + wetSignal * mix;
            }
        }
    }

    void reset() override
    {
        delayLine.reset();
        std::fill(std::begin(lastOutput), std::end(lastOutput), 0.0f);
    }

    void configure(const YAML::Node& config) override
    {
        if (config["time"]) time = config["time"].as<float>();
        if (config["feedback"]) feedback = config["feedback"].as<float>();
        if (config["mix"]) mix = config["mix"].as<float>();
        updateDelay();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("time")) time = config.getProperty("time");
        if (config.hasProperty("feedback")) feedback = config.getProperty("feedback");
        if (config.hasProperty("mix")) mix = config.getProperty("mix");
        updateDelay();
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Time (ms)", time, 0.0f, 2000.0f, [this](float v) { time=v; updateDelay(); } },
            { "Feedback", feedback, 0.0f, 0.95f, [this](float v) { feedback=v; } },
            { "Mix", mix, 0.0f, 1.0f, [this](float v) { mix=v; } }
        };
    }

    std::string getName() const override { return "Delay"; }

private:
    void updateDelay()
    {
        if (sampleRate > 0)
            delayLine.setDelay(time * (float)sampleRate / 1000.0f);
    }

    juce::dsp::DelayLine<float> delayLine { 96000 };
    double sampleRate = 44100.0;
    float time = 500.0f;
    float feedback = 0.5f;
    float mix = 0.5f;

    float lastOutput[2] = { 0.0f, 0.0f }; // Simple feedback storage for stereo
};
