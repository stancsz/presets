#pragma once
#include "../AudioEffect.h"

class ReverbEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        reverb.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        reverb.process(context);
    }

    void reset() override
    {
        reverb.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        auto params = reverb.getParameters();
        
        if (config.hasProperty("room_size") || config.getChildWithName("room_size").isValid())
            params.roomSize = getParameterValue(config, "room_size", 0.5f);
            
        if (config.hasProperty("damping") || config.getChildWithName("damping").isValid())
            params.damping = getParameterValue(config, "damping", 0.5f);
            
        if (config.hasProperty("wet") || config.getChildWithName("wet").isValid())
            params.wetLevel = getParameterValue(config, "wet", 0.33f);
            
        if (config.hasProperty("dry") || config.getChildWithName("dry").isValid())
            params.dryLevel = getParameterValue(config, "dry", 0.4f);
            
        if (config.hasProperty("width") || config.getChildWithName("width").isValid())
            params.width = getParameterValue(config, "width", 1.0f);
            
        reverb.setParameters(params);
    }

private:
    juce::dsp::Reverb reverb;
};
