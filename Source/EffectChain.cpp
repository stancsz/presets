#include "EffectChain.h"
#include "Effects/GainEffect.h"
#include "Effects/FilterEffect.h"
#include "Effects/CompressorEffect.h"
#include "Effects/LimiterEffect.h"
#include "Effects/ReverbEffect.h"
#include "Effects/ChorusEffect.h"
#include "Effects/DistortionEffect.h"
#include "Effects/PhaserEffect.h"
#include "Effects/DelayEffect.h"
#include "Effects/NoiseGateEffect.h"
#include "Effects/LadderFilterEffect.h"
#include "Effects/PannerEffect.h"
#include <yaml-cpp/yaml.h>

namespace {
    juce::ValueTree yamlToValueTree(const YAML::Node& node, const juce::Identifier& name = "Effect")
    {
        juce::ValueTree tree(name);
        
        if (node.IsScalar())
        {
            tree.setProperty("value", node.as<std::string>(), nullptr);
        }
        else if (node.IsSequence())
        {
            for (const auto& item : node)
            {
                tree.addChild(yamlToValueTree(item, "Item"), -1, nullptr);
            }
        }
        else if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                auto key = it->first.as<std::string>();
                auto val = it->second;
                
                if (val.IsScalar())
                {
                    // simple heuristic for types
                    try {
                        tree.setProperty(key, val.as<float>(), nullptr);
                    } catch(...) {
                        tree.setProperty(key, val.as<std::string>(), nullptr);
                    }
                }
                else
                {
                    // Recurse for Maps and Sequences
                    tree.addChild(yamlToValueTree(val, key), -1, nullptr);
                }
            }
        }
        return tree;
    }
}

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
    if (type == "Reverb") return std::make_unique<ReverbEffect>();
    if (type == "Chorus") return std::make_unique<ChorusEffect>();
    if (type == "Distortion") return std::make_unique<DistortionEffect>();
    if (type == "Phaser") return std::make_unique<PhaserEffect>();
    if (type == "Delay") return std::make_unique<DelayEffect>();
    if (type == "NoiseGate" || type == "Gate") return std::make_unique<NoiseGateEffect>();
    if (type == "LadderFilter" || type == "MoogFilter") return std::make_unique<LadderFilterEffect>();
    if (type == "Panner" || type == "Pan") return std::make_unique<PannerEffect>();
    return nullptr;
}

juce::Result EffectChain::loadFromValueTree(const juce::ValueTree& tree)
{
    juce::ScopedLock sl(updateLock);
    auto newEffects = std::make_shared<EffectList>();

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
    currentConfig = tree.createCopy();
    return juce::Result::ok();
}

juce::Result EffectChain::loadFromYaml(const juce::String& yamlString)
{
    try {
        YAML::Node root = YAML::Load(yamlString.toStdString());

        if (!root.IsSequence())
            return juce::Result::fail("YAML root must be a sequence/list.");

        juce::ValueTree chain("Chain");
        for (const auto& node : root)
        {
            if (node["type"])
            {
                chain.addChild(yamlToValueTree(node), -1, nullptr);
            }
        }
        
        return loadFromValueTree(chain);
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

juce::Result EffectChain::loadFromJson(const juce::String& jsonString)
{
    auto result = juce::JSON::parse(jsonString);
    if (result.isArray())
    {
        juce::ValueTree chain("Chain");
        for (int i = 0; i < result.size(); ++i)
        {
            auto item = result[i];
            if (item.isObject())
            {
                juce::ValueTree effectTree("Effect");
                auto* obj = item.getDynamicObject();
                auto props = obj->getProperties();
                for (auto& prop : props)
                {
                    effectTree.setProperty(prop.name, prop.value, nullptr);
                }
                chain.addChild(effectTree, -1, nullptr);
            }
        }
        return loadFromValueTree(chain);
    }
    return juce::Result::fail("JSON must be an array.");
}

juce::Result EffectChain::loadFromXml(const juce::String& xmlString)
{
    auto xml = juce::XmlDocument::parse(xmlString);
    if (xml)
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid())
        {
            return loadFromValueTree(tree);
        }
    }
    return juce::Result::fail("Failed to parse XML.");
}
