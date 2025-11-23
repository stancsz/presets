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



    void configure(const juce::ValueTree& config) override
    {
        if (config.hasProperty("type")) type = config.getProperty("type").toString().toStdString();
        if (config.hasProperty("frequency")) frequency = config.getProperty("frequency");
        if (config.hasProperty("q")) q = config.getProperty("q");

        updateCoefficients();
    }

private:
    void updateCoefficients()
    {
        if (sampleRate <= 0) return;

        // ProcessorDuplicator holds a pointer to the state.
        // We must update the state it is pointing to, or update the pointer itself.
        // Since 'state' is the variable we passed to the constructor, and it's a Ptr (ReferenceCountedObjectPtr),
        // we can assign a new object to it. However, ProcessorDuplicator keeps its OWN copy of the smart pointer.
        // Wait, ProcessorDuplicator<Filter, StateType> stores 'StateType state'.
        // If StateType is a pointer (Coefficients::Ptr), then it stores a pointer.
        // But if we change 'this->state', 'filter.state' is NOT updated because it's a copy of the pointer.

        // Correct approach: Update 'filter.state' directly.

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

    // Use ::Ptr type for the state so updates to 'state' are seen by 'filter'
    typename juce::dsp::IIR::Coefficients<float>::Ptr state = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 1000.0f);
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>::Ptr> filter;

    double sampleRate = 0;
    std::string type = "LowPass";
    float frequency = 1000.0f;
    float q = 0.707f;
};
