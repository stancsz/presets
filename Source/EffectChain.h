#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include "AudioEffect.h"
#include <vector>
#include <memory>
#include <atomic>
#include <string>

class EffectChain
{
public:
    EffectChain();
    ~EffectChain();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Rebuilds the chain from a YAML string
    juce::Result loadFromYaml(const juce::String& yamlString);

    // Rebuilds the chain from a JSON string
    juce::Result loadFromJson(const juce::String& jsonString);

    // Rebuilds the chain from a JUCE XML string
    juce::Result loadFromXml(const juce::String& xmlString);

    // Core loader: Rebuilds the chain from a JUCE ValueTree
    juce::Result loadFromValueTree(const juce::ValueTree& tree);

    // Helper to create effect by type name
    static std::unique_ptr<AudioEffect> createEffect(const std::string& type);

private:
    struct Node;
    using NodePtr = std::unique_ptr<Node>;

    // Simple double buffering approach for thread safety without locking audio thread
    std::shared_ptr<Node> activeRoot;

    juce::dsp::ProcessSpec currentSpec { 44100.0, 512, 2 };

    // Lock only for updating the config (write side), not for reading in process
    juce::CriticalSection updateLock;

    juce::ValueTree getCurrentConfig() const { return currentConfig; }

private:
    juce::ValueTree currentConfig;
};
