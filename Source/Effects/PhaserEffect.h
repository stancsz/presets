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

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("rate") || config.getChildWithName("rate").isValid())
            phaser.setRate(getParameterValue(config, "rate", 1.0f));
            
        if (config.hasProperty("depth") || config.getChildWithName("depth").isValid())
            phaser.setDepth(getParameterValue(config, "depth", 0.5f));
            
        if (config.hasProperty("frequency") || config.getChildWithName("frequency").isValid())
            phaser.setCentreFrequency(getParameterValue(config, "frequency", 1000.0f));
            
        if (config.hasProperty("feedback") || config.getChildWithName("feedback").isValid())
            phaser.setFeedback(getParameterValue(config, "feedback", 0.0f));
            
        if (config.hasProperty("mix") || config.getChildWithName("mix").isValid())
            phaser.setMix(getParameterValue(config, "mix", 0.5f));
    }

private:
    juce::dsp::Phaser<float> phaser;
};
