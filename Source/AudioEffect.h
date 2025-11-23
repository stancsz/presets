#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_data_structures/juce_data_structures.h>
#include <yaml-cpp/yaml.h>
#include <functional>
#include <string>
#include <vector>

/**
 * Structure defining a controllable parameter.
 */
struct EffectParameter
{
    std::string name;
    float value;
    float min;
    float max;
    std::function<void(float)> setter;
};

/**
 * Base class for all modular effects.
 */
class AudioEffect
{
public:
    virtual ~AudioEffect() = default;

    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process(const juce::dsp::ProcessContextReplacing<float>& context) = 0;
    virtual void reset() = 0;

    // Configure the effect from a YAML node (parameters)
    virtual void configure(const YAML::Node& config) = 0;

    // Configure from JUCE ValueTree (optional, for XML support)
    virtual void configure(const juce::ValueTree& config) = 0;

    // Return a list of parameters for the GUI
    virtual std::vector<EffectParameter> getParameters() = 0;

    // Return the name/type of the effect
    virtual std::string getName() const = 0;
};
