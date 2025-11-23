#include "EffectChain.h"
#include "Effects/GainEffect.h"
#include "Effects/FilterEffect.h"
#include "Effects/CompressorEffect.h"
#include "Effects/LimiterEffect.h"
#include "Effects/DelayEffect.h"
#include "Effects/DriveEffect.h"
#include "Effects/PhaserEffect.h"
#include "Effects/ChorusEffect.h"
#include "Effects/ReverbEffect.h"
#include <yaml-cpp/yaml.h>

EffectChain::EffectChain()
{
    activeEffects = std::make_shared<EffectList>();
}

EffectChain::~EffectChain()
{
}

void EffectChain::prepare(const juce::dsp::ProcessSpec& spec)
{
    juce::ScopedLock sl(updateLock);
    currentSpec = spec;

    auto currentList = std::atomic_load(&activeEffects);
    if (currentList)
    {
        for (auto& effect : *currentList)
        {
            effect->prepare(spec);
        }
    }
}

void EffectChain::process(juce::AudioBuffer<float>& buffer)
{
    auto currentList = std::atomic_load(&activeEffects);

    if (currentList)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        for (auto& effect : *currentList)
        {
            effect->process(context);
        }
    }
}

void EffectChain::reset()
{
    juce::ScopedLock sl(updateLock);
    auto currentList = std::atomic_load(&activeEffects);
    if (currentList)
    {
        for (auto& effect : *currentList)
        {
            effect->reset();
        }
    }
}

std::unique_ptr<AudioEffect> EffectChain::createEffect(const std::string& type)
{
    if (type == "Gain") return std::make_unique<GainEffect>();
    if (type == "Filter" || type == "EQ") return std::make_unique<FilterEffect>();
    if (type == "Compressor") return std::make_unique<CompressorEffect>();
    if (type == "Limiter") return std::make_unique<LimiterEffect>();
    if (type == "Delay") return std::make_unique<DelayEffect>();
    if (type == "Drive" || type == "Distortion") return std::make_unique<DriveEffect>();
    if (type == "Phaser") return std::make_unique<PhaserEffect>();
    if (type == "Chorus") return std::make_unique<ChorusEffect>();
    if (type == "Reverb") return std::make_unique<ReverbEffect>();
    return nullptr;
}

juce::Result EffectChain::loadFromYaml(const juce::String& yamlString)
{
    juce::ScopedLock sl(updateLock);

    auto newEffects = std::make_shared<EffectList>();

    try {
        YAML::Node root = YAML::Load(yamlString.toStdString());

        if (!root.IsSequence())
            return juce::Result::fail("YAML root must be a sequence/list.");

        for (const auto& node : root)
        {
            if (node["type"])
            {
                std::string type = node["type"].as<std::string>();
                auto effect = createEffect(type);
                if (effect)
                {
                    effect->configure(node);
                    effect->prepare(currentSpec);
                    newEffects->push_back(std::move(effect));
                }
                else
                {
                     return juce::Result::fail("Unknown effect type: " + type);
                }
            }
        }

        std::atomic_store(&activeEffects, newEffects);

        // Notify listeners on message thread if possible, but we are here likely on message thread
        listeners.call(&Listener::chainChanged);

        return juce::Result::ok();
    }
    catch (const YAML::Exception& e)
    {
        return juce::Result::fail(juce::String("YAML Error: ") + e.what());
    }
    catch (...)
    {
        return juce::Result::fail("Unknown error parsing YAML.");
    }
}

juce::Result EffectChain::loadFromXml(const juce::String& xmlString)
{
    juce::ScopedLock sl(updateLock);

    auto newEffects = std::make_shared<EffectList>();

    auto xml = juce::XmlDocument::parse(xmlString);
    if (xml)
    {
        auto tree = juce::ValueTree::fromXml(*xml);

        if (!tree.isValid())
             return juce::Result::fail("Invalid XML.");

        for (const auto& child : tree)
        {
             if (child.hasProperty("type"))
             {
                 std::string type = child.getProperty("type").toString().toStdString();
                 auto effect = createEffect(type);
                 if (effect)
                 {
                     effect->configure(child);
                     effect->prepare(currentSpec);
                     newEffects->push_back(std::move(effect));
                 }
                 else
                 {
                     return juce::Result::fail("Unknown effect type: " + type);
                 }
             }
        }

        std::atomic_store(&activeEffects, newEffects);

        listeners.call(&Listener::chainChanged);

        return juce::Result::ok();
    }

    return juce::Result::fail("Failed to parse XML.");
}

void EffectChain::visitEffects(std::function<void(AudioEffect*)> visitor)
{
    // Grab a shared_ptr copy to ensure the list keeps existing during iteration
    auto currentList = std::atomic_load(&activeEffects);
    if (currentList)
    {
        for (auto& effect : *currentList)
        {
            visitor(effect.get());
        }
    }
}
