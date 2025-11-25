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

// ============================
// Internal Node Graph Types
// ============================

struct EffectChain::Node
{
    virtual ~Node() = default;
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;
};

namespace {
    using NodePtr = std::unique_ptr<EffectChain::Node>;

    struct EffectNode : public EffectChain::Node
    {
        explicit EffectNode(std::unique_ptr<AudioEffect> e)
            : effect(std::move(e)) {}

        void prepare(const juce::dsp::ProcessSpec& spec) override
        {
            if (effect)
                effect->prepare(spec);
        }

        void process(juce::AudioBuffer<float>& buffer) override
        {
            if (!effect)
                return;

            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            effect->process(context);
        }

        void reset() override
        {
            if (effect)
                effect->reset();
        }

        std::unique_ptr<AudioEffect> effect;
    };

    struct GroupNode : public EffectChain::Node
    {
        enum class Mode { Series, Parallel };

        GroupNode() = default;

        void prepare(const juce::dsp::ProcessSpec& spec) override
        {
            processSpec = spec;

            const auto numChannels = static_cast<int>(spec.numChannels);
            const auto maxBlock = static_cast<int>(spec.maximumBlockSize);

            tempBuffer.setSize(numChannels, maxBlock);
            mixBuffer.setSize(numChannels, maxBlock);

            for (auto& child : children)
                child->prepare(spec);
        }

        void process(juce::AudioBuffer<float>& buffer) override
        {
            const int passes = repeat > 0 ? repeat : 1;

            if (mode == Mode::Series || children.empty())
            {
                for (int pass = 0; pass < passes; ++pass)
                    for (auto& child : children)
                        child->process(buffer);
                return;
            }

            const int numChannels = buffer.getNumChannels();
            const int numSamples  = buffer.getNumSamples();

            mixBuffer.clear();

            for (auto& child : children)
            {
                tempBuffer.makeCopyOf(buffer, true);
                child->process(tempBuffer);

                for (int ch = 0; ch < numChannels; ++ch)
                    mixBuffer.addFrom(ch, 0, tempBuffer, ch, 0, numSamples);
            }

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.copyFrom(ch, 0, mixBuffer, ch, 0, numSamples);

            // Additional passes just re-process the mixed signal
            for (int pass = 1; pass < passes; ++pass)
            {
                for (auto& child : children)
                {
                    tempBuffer.makeCopyOf(buffer, true);
                    child->process(tempBuffer);

                    for (int ch = 0; ch < numChannels; ++ch)
                        mixBuffer.addFrom(ch, 0, tempBuffer, ch, 0, numSamples);
                }

                for (int ch = 0; ch < numChannels; ++ch)
                    buffer.copyFrom(ch, 0, mixBuffer, ch, 0, numSamples);
            }
        }

        void reset() override
        {
            for (auto& child : children)
                child->reset();
        }

        Mode mode { Mode::Series };
        int repeat { 1 };
        std::vector<NodePtr> children;

        juce::dsp::ProcessSpec processSpec {};
        juce::AudioBuffer<float> tempBuffer;
        juce::AudioBuffer<float> mixBuffer;
    };

    juce::ValueTree yamlToValueTree(const YAML::Node& node, const juce::Identifier& name = "Effect")
    {
        juce::ValueTree tree(name);
        
        if (node.IsScalar())
        {
            tree.setProperty("value", juce::String(node.as<std::string>()), nullptr);
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
                const auto& val = it->second;
                
                if (val.IsScalar())
                {
                    // simple heuristic for types
                    bool forceString = (key == "mode" || key == "type" || key == "ui" || key == "style");
                    
                    if (forceString)
                    {
                        tree.setProperty(juce::Identifier(key), juce::String(val.as<std::string>()), nullptr);
                    }
                    else
                    {
                        try
                        {
                            tree.setProperty(juce::Identifier(key), juce::var(val.as<float>()), nullptr);
                        }
                        catch (...)
                        {
                            tree.setProperty(juce::Identifier(key), juce::String(val.as<std::string>()), nullptr);
                        }
                    }
                }
                else
                {
                    // Special handling for "children" to support nested groups/effect graphs
                    if (key == "children" && val.IsSequence())
                    {
                        for (const auto& item : val)
                            tree.addChild(yamlToValueTree(item, "Effect"), -1, nullptr);
                    }
                    else
                    {
                        // Recurse for Maps and other Sequences
                        tree.addChild(yamlToValueTree(val, juce::Identifier(key)), -1, nullptr);
                    }
                }
            }
        }
        return tree;
    }

    NodePtr buildNodeFromValueTree(const juce::ValueTree& tree)
    {
        if (!tree.hasProperty("type"))
            return nullptr;

        auto typeStr = tree.getProperty("type").toString();

        // Group node: can be used for series/parallel, with optional repeat
        if (typeStr == "Group")
        {
            auto group = std::make_unique<GroupNode>();

            auto modeStr = tree.getProperty("mode", "series").toString().toLowerCase();
            if (modeStr == "parallel")
                group->mode = GroupNode::Mode::Parallel;
            else
                group->mode = GroupNode::Mode::Series;

            if (tree.hasProperty("repeat"))
                group->repeat = static_cast<int>(tree.getProperty("repeat"));

            for (int i = 0; i < tree.getNumChildren(); ++i)
            {
                auto childTree = tree.getChild(i);
                if (!childTree.isValid())
                    continue;

                auto childNode = buildNodeFromValueTree(childTree);
                if (childNode)
                    group->children.push_back(std::move(childNode));
            }

            return group;
        }

        // Leaf effect node
        auto effect = EffectChain::createEffect(typeStr.toStdString());
        if (!effect)
            return nullptr;

        effect->configure(tree);
        return std::make_unique<EffectNode>(std::move(effect));
    }
}

EffectChain::EffectChain()
{
}

EffectChain::~EffectChain()
{
}

void EffectChain::prepare(const juce::dsp::ProcessSpec& spec)
{
    juce::ScopedLock sl(updateLock);
    currentSpec = spec;

    auto root = std::atomic_load(&activeRoot);
    if (root)
        root->prepare(spec);
}

void EffectChain::process(juce::AudioBuffer<float>& buffer)
{
    auto root = std::atomic_load(&activeRoot);
    if (root)
        root->process(buffer);
}

void EffectChain::reset()
{
    juce::ScopedLock sl(updateLock);
    auto root = std::atomic_load(&activeRoot);
    if (root)
        root->reset();
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

    // Build a root group (series) that contains all top-level children
    auto rootGroup = std::make_shared<GroupNode>();
    rootGroup->mode = GroupNode::Mode::Series;

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        auto child = tree.getChild(i);
        if (!child.isValid())
            continue;

        auto node = buildNodeFromValueTree(child);
        if (!node)
        {
            auto typeStr = child.getProperty("type").toString();
            return juce::Result::fail("Unknown or invalid effect/group type: " + typeStr);
        }

        rootGroup->children.push_back(std::move(node));
    }

    rootGroup->prepare(currentSpec);

    std::atomic_store(&activeRoot, std::static_pointer_cast<Node>(rootGroup));
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
