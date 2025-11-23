#pragma once
#include "../AudioEffect.h"

class DelayEffect : public AudioEffect
{
public:
    DelayEffect()
    {
        delayLine.setMaximumDelayInSamples(192000); // Max 2-4 seconds depending on SR
    }

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        delayLine.prepare(spec);
        
        // Reset smoothing
        delayTime.reset(sampleRate, 0.05);
        feedback.reset(sampleRate, 0.05);
        mix.reset(sampleRate, 0.05);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        size_t numChannels = inputBlock.getNumChannels();
        size_t numSamples = inputBlock.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* src = inputBlock.getChannelPointer(ch);
            auto* dst = outputBlock.getChannelPointer(ch);

            for (size_t i = 0; i < numSamples; ++i)
            {
                float dry = src[i];
                
                // Get delayed sample
                float currentDelay = delayTime.getNextValue() * (float)sampleRate;
                float delayed = delayLine.popSample((int)ch, currentDelay);
                
                // Calculate output
                float wet = delayed;
                float out = dry * (1.0f - mix.getNextValue()) + wet * mix.getNextValue();
                
                // Feedback
                float fb = feedback.getNextValue();
                delayLine.pushSample((int)ch, dry + delayed * fb);
                
                dst[i] = out;
            }
        }
    }

    void reset() override
    {
        delayLine.reset();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("time") || config.getChildWithName("time").isValid())
            delayTime.setTargetValue(getParameterValue(config, "time", 0.5f));
            
        if (config.hasProperty("feedback") || config.getChildWithName("feedback").isValid())
            feedback.setTargetValue(getParameterValue(config, "feedback", 0.3f));
            
        if (config.hasProperty("mix") || config.getChildWithName("mix").isValid())
            mix.setTargetValue(getParameterValue(config, "mix", 0.5f));
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    double sampleRate = 44100.0;
    
    juce::SmoothedValue<float> delayTime { 0.5f };
    juce::SmoothedValue<float> feedback { 0.3f };
    juce::SmoothedValue<float> mix { 0.5f };
};
