#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_data_structures/juce_data_structures.h>

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

    // Configure from JUCE ValueTree
    virtual void configure(const juce::ValueTree& config) = 0;

protected:
    // Helper to extract a float value whether it's a direct property or a nested "value" property
    static float getParameterValue(const juce::ValueTree& config, const juce::Identifier& id, float defaultValue = 0.0f)
    {
        // Case 1: Simple property (e.g. gain_db: -6.0)
        if (config.hasProperty(id))
            return config.getProperty(id);

        // Case 2: Nested child (e.g. gain_db: { value: -6.0, ui: knob })
        auto child = config.getChildWithName(id);
        if (child.isValid() && child.hasProperty("value"))
            return child.getProperty("value");

        // Case 3: Nested child with "default" fallback
        if (child.isValid() && child.hasProperty("default"))
             return child.getProperty("default");

        return defaultValue;
    }
};
