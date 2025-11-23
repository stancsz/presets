#pragma once
#include "../AudioEffect.h"

class FilterEffect : public AudioEffect
{
public:
    FilterEffect() : filter(state)
    {
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

    void configure(const YAML::Node& config) override
    {
        if (config["type"]) type = config["type"].as<std::string>();
        if (config["frequency"]) frequency = config["frequency"].as<float>();
        if (config["q"]) q = config["q"].as<float>();

        updateCoefficients();
    }

    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("type")) type = config.getProperty("type").toString().toStdString();
        if (config.hasProperty("frequency")) frequency = config.getProperty("frequency");
        if (config.hasProperty("q")) q = config.getProperty("q");

        updateCoefficients();
    }

    std::vector<EffectParameter> getParameters() override
    {
        return {
            { "Frequency", frequency, 20.0f, 20000.0f, [this](float v) { frequency = v; updateCoefficients(); } },
            { "Q", q, 0.1f, 10.0f, [this](float v) { q = v; updateCoefficients(); } }
        };
    }

    std::string getName() const override { return "Filter (" + type + ")"; }

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
            *filter.state = *newCoeffs;
        }
    }

    typename juce::dsp::IIR::Coefficients<float>::Ptr state = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 1000.0f);
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>::Ptr> filter;

    double sampleRate = 0;
    std::string type = "LowPass";
    float frequency = 1000.0f;
    float q = 0.707f;
};
