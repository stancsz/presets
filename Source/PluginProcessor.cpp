#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PresetEngineAudioProcessor::PresetEngineAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

PresetEngineAudioProcessor::~PresetEngineAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PresetEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    // We keep these dummy parameters or remove them.
    // The user wants the structure to be defined by code.
    // We can expose a "Code" parameter as a big string, but APVTS params are usually float/bool/choice.
    // The code/state is better handled in getStateInformation.

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PresetEngineAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PresetEngineAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PresetEngineAudioProcessor::producesMidi() const
{
   #if JucePlugin_WantsMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PresetEngineAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PresetEngineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PresetEngineAudioProcessor::getNumPrograms()
{
    return 1;
}

int PresetEngineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PresetEngineAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PresetEngineAudioProcessor::getProgramName (int index)
{
    return {};
}

void PresetEngineAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PresetEngineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    effectChain.prepare(spec);
}

void PresetEngineAudioProcessor::releaseResources()
{
    effectChain.reset();
}

bool PresetEngineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PresetEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    effectChain.process(buffer);
}

//==============================================================================
bool PresetEngineAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PresetEngineAudioProcessor::createEditor()
{
    return new PresetEngineAudioProcessorEditor (*this);
}

//==============================================================================
void PresetEngineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the current config.
    // For now, we assume the config string is stored in a member or we reconstruct it.
    // Actually, we should store the 'source code' that generated the chain.
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString(currentConfigCode);
}

void PresetEngineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, sizeInBytes, false);
    juce::String config = stream.readString();

    loadConfig(config);
}

juce::Result PresetEngineAudioProcessor::loadConfig(const juce::String& config)
{
    currentConfigCode = config;

    // Detect format
    juce::String trimmed = config.trim();
    if (trimmed.startsWith("<"))
        return effectChain.loadFromXml(config);
    else if (trimmed.startsWith("{") || trimmed.startsWith("["))
        return effectChain.loadFromJson(config);
    else
        return effectChain.loadFromYaml(config);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PresetEngineAudioProcessor();
}
