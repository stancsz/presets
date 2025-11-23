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



    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("threshold")) compressor.setThreshold(config.getProperty("threshold"));
        if (config.hasProperty("ratio")) compressor.setRatio(config.getProperty("ratio"));
        if (config.hasProperty("attack")) compressor.setAttack(config.getProperty("attack"));
        if (config.hasProperty("release")) compressor.setRelease(config.getProperty("release"));
    }

private:
    juce::dsp::Compressor<float> compressor;
};
