#pragma once
#include "../AudioEffect.h"
#include <string>

class FilterEffect : public AudioEffect
{
public:
    FilterEffect() : filter(state)
    {
        // Default state
        state = new juce::dsp::IIR::Coefficients<float>(*juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 1000.0f));
        updateCoefficients();
    }

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        filter.prepare(spec);
        updateCoefficients();
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
        // Support both "type" (legacy/YAML) and "mode" (JSON friendly) for filter type
        if (config.hasProperty("mode")) 
            type = config.getProperty("mode").toString().toStdString();
        else if (config.hasProperty("type") && config.getProperty("type").toString() != "Filter") 
            type = config.getProperty("type").toString().toStdString();

        if (config.hasProperty("frequency") || config.getChildWithName("frequency").isValid()) 
            frequency = getParameterValue(config, "frequency", 1000.0f);
            
        if (config.hasProperty("q") || config.getChildWithName("q").isValid()) 
            q = getParameterValue(config, "q", 0.707f);

        updateCoefficients();
    }

private:
    void updateCoefficients()
    {
        if (sampleRate <= 0) return;

        typename juce::dsp::IIR::Coefficients<float>::Ptr newCoeffs;

        if (type == "LowPass")
            newCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, frequency, q);
        else if (type == "HighPass")
            newCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, frequency, q);
        else if (type == "BandPass")
            newCoeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, frequency, q);

        if (newCoeffs)
        {
            *state = *newCoeffs;
        }
    }

    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    Coefficients::Ptr state;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, Coefficients> filter;

    double sampleRate = 0;
    std::string type = "LowPass";
    float frequency = 1000.0f;
    float q = 0.707f;
};
