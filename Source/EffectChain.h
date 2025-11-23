#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include "AudioEffect.h"
#include <vector>
#include <memory>
#include <atomic>
#include <functional>

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

    // Rebuilds the chain from a JUCE XML string
    juce::Result loadFromXml(const juce::String& xmlString);

    // Helper to create effect by type name
    static std::unique_ptr<AudioEffect> createEffect(const std::string& type);

    // Visit the current effects (for UI).
    // Safe to call from message thread if we assume the effects themselves aren't being deleted
    // concurrently in a way that invalidates this iteration (activeEffects is a shared_ptr).
    void visitEffects(std::function<void(AudioEffect*)> visitor);

    // Listeners for chain updates
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void chainChanged() = 0;
    };

    void addListener(Listener* l) { listeners.add(l); }
    void removeListener(Listener* l) { listeners.remove(l); }

private:
    // Simple double buffering approach for thread safety without locking audio thread
    // Note: In a strict real-time context, the destruction of the old list (when unique_ptrs go out of scope)
    // happens on the thread performing the swap or the last thread to release the shared_ptr.
    // Here, loadFromYaml/Xml happens on the message thread usually, so destruction happens there, which is fine.
    // If the audio thread holds the last reference, destruction happens there, which is technically not RT-safe.
    using EffectList = std::vector<std::unique_ptr<AudioEffect>>;

    std::shared_ptr<EffectList> activeEffects;

    juce::dsp::ProcessSpec currentSpec { 44100.0, 512, 2 };

    // Lock only for updating the config (write side), not for reading in process
    juce::CriticalSection updateLock;

    juce::ListenerList<Listener> listeners;
};
