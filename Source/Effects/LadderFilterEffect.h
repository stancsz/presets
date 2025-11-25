#pragma once
#include "../AudioEffect.h"
#include <string>

class LadderFilterEffect : public AudioEffect
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        filter.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        filter.process(context);
    }

    void reset() override
    {
        filter.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("frequency") || config.getChildWithName("frequency").isValid())
            filter.setCutoffFrequencyHz(getParameterValue(config, "frequency", 1000.0f));
            
        if (config.hasProperty("resonance") || config.getChildWithName("resonance").isValid())
            filter.setResonance(getParameterValue(config, "resonance", 0.0f));
            
        if (config.hasProperty("drive") || config.getChildWithName("drive").isValid())
            filter.setDrive(getParameterValue(config, "drive", 1.0f));

        if (config.hasProperty("mode"))
        {
            juce::String mode = config.getProperty("mode").toString();
            if (mode.equalsIgnoreCase("LP12")) filter.setMode(juce::dsp::LadderFilterMode::LPF12);
            else if (mode.equalsIgnoreCase("LP24")) filter.setMode(juce::dsp::LadderFilterMode::LPF24);
            else if (mode.equalsIgnoreCase("HP12")) filter.setMode(juce::dsp::LadderFilterMode::HPF12);
            else if (mode.equalsIgnoreCase("HP24")) filter.setMode(juce::dsp::LadderFilterMode::HPF24);
            else if (mode.equalsIgnoreCase("BP12")) filter.setMode(juce::dsp::LadderFilterMode::BPF12);
            else if (mode.equalsIgnoreCase("BP24")) filter.setMode(juce::dsp::LadderFilterMode::BPF24);
        }
    }

private:
    juce::dsp::LadderFilter<float> filter;
};
