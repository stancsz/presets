
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <memory>

//==============================================================================
// Component to render a single parameter
class DynamicParameterComponent : public juce::Component
{
public:
    static juce::String formatParamLabel(const juce::String& name)
    {
        juce::String s = name;
        s = s.replace("_", " ");
        
        // Handle units
        if (s.endsWithIgnoreCase(" db")) s = s.dropLastCharacters(3) + " (dB)";
        else if (s.endsWithIgnoreCase(" hz")) s = s.dropLastCharacters(3) + " (Hz)";
        else if (s.endsWithIgnoreCase(" ms")) s = s.dropLastCharacters(3) + " (ms)";
        
        // Title Case
        bool capitalizeNext = true;
        juce::String result;
        for (int i = 0; i < s.length(); ++i)
        {
            auto c = s[i];
            if (capitalizeNext && std::isalpha(c))
            {
                result += (juce::juce_wchar)std::toupper(c);
                capitalizeNext = false;
            }
            else
            {
                result += c;
                if (std::isspace(c)) capitalizeNext = true;
            }
        }
        
        return result;
    }

    DynamicParameterComponent(const juce::String& name, const juce::ValueTree& config)
        : paramName(name)
    {
        juce::String uiType = config.getProperty("ui", "Slider").toString();
        
        // Label (except for Button/Label types where it's redundant or handled differently)
        if (!uiType.equalsIgnoreCase("Button") && !uiType.equalsIgnoreCase("ToggleButton") && !uiType.equalsIgnoreCase("Label"))
        {
            addAndMakeVisible(label);
            label.setText(formatParamLabel(name), juce::dontSendNotification); // Nicely formatted
            label.setJustificationType(juce::Justification::centred);
            label.setFont(juce::Font(10.0f, juce::Font::bold)); // Smaller, bolder
            label.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
        }

        // Control Factory
        if (uiType.equalsIgnoreCase("Slider"))
        {
            auto* s = new juce::Slider();
            component.reset(s);
            
            juce::String style = config.getProperty("style", "Rotary").toString();
            if (style.equalsIgnoreCase("Linear")) s->setSliderStyle(juce::Slider::LinearHorizontal);
            else if (style.equalsIgnoreCase("LinearVertical")) s->setSliderStyle(juce::Slider::LinearVertical);
            else s->setSliderStyle(juce::Slider::RotaryVerticalDrag);
            
            s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 14);
            s->setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
            s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            
            float val = config.getProperty("value", 0.0f);
            float min = config.getProperty("min", 0.0f);
            float max = config.getProperty("max", 1.0f);
            
            if (!config.hasProperty("min") && !config.hasProperty("max"))
            {
                if (val < 0) {
                    min = std::min(-60.0f, val * 2.0f);
                    max = std::max(0.0f, std::abs(val));
                } else {
                    min = 0.0f;
                    max = std::max(1.0f, val * 2.0f);
                }
            }

            s->setRange(min, max);
            
            // 2. The Interaction Link: Value Formatting
            s->textFromValueFunction = [name](double value) {
                juce::String suffix = "";
                if (name.containsIgnoreCase("db") || name.containsIgnoreCase("gain") || name.containsIgnoreCase("threshold")) suffix = " dB";
                else if (name.containsIgnoreCase("hz") || name.containsIgnoreCase("freq")) suffix = " Hz";
                else if (name.containsIgnoreCase("ms") || name.containsIgnoreCase("time")) suffix = " ms";
                else if (name.containsIgnoreCase("ratio")) suffix = ":1";
                else if (name.containsIgnoreCase("percent") || name.containsIgnoreCase("mix") || name.containsIgnoreCase("dry") || name.containsIgnoreCase("wet")) suffix = " %";
                
                return juce::String(value, 1) + suffix;
            };
            
            s->setValue(val, juce::dontSendNotification);
        }
        else if (uiType.equalsIgnoreCase("Button") || uiType.equalsIgnoreCase("ToggleButton"))
        {
            auto* b = new juce::ToggleButton();
            component.reset(b);
            b->setButtonText(formatParamLabel(name));
            bool val = config.getProperty("value", false);
            b->setToggleState(val, juce::dontSendNotification);
        }
        else if (uiType.equalsIgnoreCase("ComboBox"))
        {
            auto* c = new juce::ComboBox();
            component.reset(c);
            
            // Parse options from children named "Item"
            int id = 1;
            for (const auto& child : config)
            {
                if (child.getType().toString() == "Item")
                {
                    c->addItem(child.getProperty("value").toString(), id++);
                }
            }
            
            // Fallback if no items
            if (c->getNumItems() == 0)
            {
                c->addItem("Default", 1);
            }

            // Set selected based on value (string or int)
            auto val = config.getProperty("value");
            if (val.isString())
            {
                // Try to find item with this text
                bool found = false;
                for (int i=0; i<c->getNumItems(); ++i)
                {
                    if (c->getItemText(i).equalsIgnoreCase(val.toString()))
                    {
                        c->setSelectedItemIndex(i);
                        found = true;
                        break;
                    }
                }
                if (!found) c->setSelectedId(1);
            }
            else
            {
                c->setSelectedId((int)val > 0 ? (int)val : 1);
            }
        }
        else if (uiType.equalsIgnoreCase("Label"))
        {
            auto* l = new juce::Label();
            component.reset(l);
            l->setText(config.getProperty("value").toString(), juce::dontSendNotification);
            l->setJustificationType(juce::Justification::centred);
            l->setColour(juce::Label::textColourId, juce::Colours::cyan);
        }
        else if (uiType.equalsIgnoreCase("Meter"))
        {
            auto* p = new juce::ProgressBar(dummyProgress);
            component.reset(p);
            // Static value for now
            dummyProgress = config.getProperty("value", 0.5f);
        }
        else
        {
            // Default
            auto* l = new juce::Label();
            component.reset(l);
            l->setText(config.getProperty("value").toString(), juce::dontSendNotification);
        }

        if (component)
            addAndMakeVisible(component.get());
    }

    void resized() override
    {
        auto area = getLocalBounds();
        if (label.isVisible())
        {
            label.setBounds(area.removeFromBottom(20));
        }
        if (component)
        {
            // Fix: Increase width for ComboBox to avoid truncation
            if (dynamic_cast<juce::ComboBox*>(component.get()))
            {
                component->setBounds(area.reduced(2).withWidth(std::max(area.getWidth(), 90)));
            }
            else
            {
                component->setBounds(area.reduced(2));
            }
        }
    }

private:
    juce::String paramName;
    juce::Label label;
    std::unique_ptr<juce::Component> component;
    double dummyProgress = 0.0;
};

//==============================================================================
// Component to render a group of parameters for one Effect
// Component to render a group of parameters for one Effect
// Component to render a group of parameters for one Effect
// Component to render a group of parameters for one Effect
class DynamicEffectComponent : public juce::Component
{
public:
    DynamicEffectComponent(const juce::ValueTree& effectTree)
    {
        // Title
        juce::String type = effectTree.getProperty("type").toString();

        effectNameLabel.setText(type.toUpperCase(), juce::dontSendNotification);
        effectNameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        effectNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00bcd4)); // Cyan
        effectNameLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(effectNameLabel);

        // Iterate properties and children to find parameters
        // 1. Properties (Simple scalars)
        for (int i = 0; i < effectTree.getNumProperties(); ++i)
        {
            auto name = effectTree.getPropertyName(i).toString();
            if (name == "type") continue;

            auto val = effectTree.getProperty(name);
            juce::ValueTree simpleConfig("Param");
            simpleConfig.setProperty("value", val, nullptr);
            
            // Heuristic for UI Type
            juce::String uiType = "Slider";
            if (name.equalsIgnoreCase("mode"))
                uiType = "ComboBox";
            else if (name.equalsIgnoreCase("bypass") || name.equalsIgnoreCase("enabled"))
                uiType = "ToggleButton";
            else if (val.isString())
                uiType = "Label"; // Fallback for strings
            
            simpleConfig.setProperty("ui", uiType, nullptr);

            // Populate options for known modes
            if (uiType == "ComboBox" && name.equalsIgnoreCase("mode"))
            {
                juce::StringArray options;
                if (type.equalsIgnoreCase("Filter"))
                {
                    options.add("LowPass"); options.add("HighPass"); options.add("BandPass");
                }
                else if (type.equalsIgnoreCase("LadderFilter"))
                {
                    options.add("LP12"); options.add("LP24"); 
                    options.add("HP12"); options.add("HP24");
                    options.add("BP12"); options.add("BP24");
                }
                else if (type.equalsIgnoreCase("Group"))
                {
                    options.add("Series"); options.add("Parallel");
                }
                
                for (const auto& opt : options)
                {
                    juce::ValueTree item("Item");
                    item.setProperty("value", opt, nullptr);
                    simpleConfig.addChild(item, -1, nullptr);
                }
            }

            auto* comp = new DynamicParameterComponent(name, simpleConfig);
            params.add(comp);
            addAndMakeVisible(comp);
        }

        // 2. Children (Complex params only, skip nested effects)
        for (const auto& child : effectTree)
        {
            if (child.hasProperty("type")) continue; // Skip nested effects

            auto name = child.getType().toString();
            auto* comp = new DynamicParameterComponent(name, child);
            params.add(comp);
            addAndMakeVisible(comp);
        }
    }

    void setIndent(int level) { indentLevel = level; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);
        int indentPx = indentLevel * 20;
        bounds.removeFromLeft((float)indentPx);

        // Background for the effect block
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Border
        g.setColour(juce::Colour(0xff3a3a3a));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(5); // Padding inside the block
        int indentPx = indentLevel * 20;
        area.removeFromLeft(indentPx);
        
        // Header
        effectNameLabel.setBounds(area.removeFromTop(25).reduced(5, 0));

        // Content
        juce::FlexBox flex;
        flex.flexWrap = juce::FlexBox::Wrap::wrap;
        flex.justifyContent = juce::FlexBox::JustifyContent::flexStart; // Or spaceBetween if we want them spread out
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        for (auto* p : params)
        {
            // 3. The Layout Link: FlexBox vs setBounds
            flex.items.add(juce::FlexItem(*p).withWidth(70.0f).withHeight(80.0f).withMargin(juce::FlexItem::Margin(5.0f)));
        }

        flex.performLayout(area);
    }

private:
    juce::Label effectNameLabel;
    juce::OwnedArray<DynamicParameterComponent> params;
    int indentLevel = 0;
};

//==============================================================================
PresetEngineAudioProcessorEditor::PresetEngineAudioProcessorEditor (PresetEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), spectrumComponent(p)
{
    // Apply custom look and feel
    setLookAndFeel(&lookAndFeel);

    setSize (900, 500); // Widescreen modern aspect

    // Header
    titleLabel.setText("Preset Engine", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00bcd4)); // Cyan title
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Nap-Tech", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(12.0f, juce::Font::italic));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    // Spectrum
    addAndMakeVisible(spectrumComponent);

    // Dynamic UI Viewport
    addAndMakeVisible(viewport);
    viewport.setScrollBarsShown(true, false);
    container.reset(new juce::Component());
    viewport.setViewedComponent(container.get(), false);

    // Code Editor Controls
    languageBox.addItem("YAML", 1);
    languageBox.addItem("JSON", 2);
    languageBox.addItem("XML", 3);
    languageBox.addItem("Python", 4);
    languageBox.setSelectedId(1);
    languageBox.onChange = [this] { presetBox.onChange(); }; // Trigger update
    addAndMakeVisible(languageBox);

    presetBox.addItem("Default", 1);
    presetBox.addItem("Vocal Strip", 2);
    presetBox.addItem("Drum Smash", 3);
    presetBox.addItem("Ethereal Space", 4);
    presetBox.addItem("Mastering Chain", 5);
    presetBox.addItem("Lo-Fi", 6);
    presetBox.addItem("Complex Group", 7);
    presetBox.setSelectedId(1);
    presetBox.onChange = [this] {
        codeEditor.setText(getPresetSource(presetBox.getSelectedId(), languageBox.getSelectedId()));
    };
    addAndMakeVisible(presetBox);

    exampleButton.setButtonText("Reset");
    exampleButton.onClick = [this] {
        presetBox.setSelectedId(1);
    };
    addAndMakeVisible(exampleButton);

    codeEditor.setMultiLine(true);
    codeEditor.setReturnKeyStartsNewLine(true);
    codeEditor.setTabKeyUsedAsCharacter(true);
    codeEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    
    if (audioProcessor.getCurrentConfig().isEmpty())
        presetBox.onChange();
    else
        codeEditor.setText(audioProcessor.getCurrentConfig());
        
    addAndMakeVisible(codeEditor);

    statusLabel.setText("Ready.", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(12.0f));
    statusLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(statusLabel);

    applyButton.setButtonText("APPLY");
    applyButton.onClick = [this] {
        juce::String configToLoad = codeEditor.getText();

        if (languageBox.getSelectedId() == 4) // Python
        {
             // Transpile "Python" to YAML
             configToLoad = parsePythonToYaml(configToLoad);
             
             if (configToLoad.isEmpty())
             {
                 statusLabel.setText("Error: No valid chain.add() calls found.", juce::dontSendNotification);
                 statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
                 return;
             }
        }

        auto result = audioProcessor.loadConfig(configToLoad);
        if (result.wasOk())
        {
            statusLabel.setText("Loaded Successfully", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
            rebuildUi();
        }
        else
        {
            statusLabel.setText("Error: " + result.getErrorMessage(), juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        }
    };
    addAndMakeVisible(applyButton);

    rebuildUi();
}

PresetEngineAudioProcessorEditor::~PresetEngineAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    viewport.setViewedComponent(nullptr, false);
}

juce::String PresetEngineAudioProcessorEditor::getPresetSource(int presetId, int languageId)
{
    // 1=YAML, 2=JSON, 3=XML, 4=Python
    // Presets: 1=Default, 2=Vocal, 3=Drum, 4=Space, 5=Master, 6=LoFi

    if (languageId == 1) // YAML
    {
        if (presetId == 2) return 
            "- type: NoiseGate\n  threshold: -60.0\n  ratio: 10.0\n"
            "- type: Filter\n  mode: HighPass\n  frequency: 80.0\n"
            "- type: Compressor\n  threshold: -20.0\n  ratio: 4.0\n  attack: 5.0\n  release: 100.0";
        if (presetId == 3) return
            "- type: Distortion\n  drive: 6.0\n"
            "- type: Compressor\n  threshold: -15.0\n  ratio: 8.0\n  attack: 1.0\n  release: 50.0\n"
            "- type: Gain\n  gain_db: 3.0";
        if (presetId == 4) return
            "- type: Delay\n  time: 0.4\n  feedback: 0.6\n  mix: 0.4\n"
            "- type: Reverb\n  room_size: 0.8\n  wet: 0.5\n"
            "- type: Phaser\n  rate: 0.2\n  depth: 0.7";
        if (presetId == 5) return
            "- type: Filter\n  mode: HighPass\n  frequency: 30.0\n"
            "- type: Compressor\n  threshold: -10.0\n  ratio: 2.0\n  attack: 30.0\n  release: 200.0\n"
            "- type: Limiter\n  threshold: -0.1\n  release: 100.0";
        if (presetId == 6) return
            "- type: Distortion\n  drive: 12.0\n"
            "- type: Filter\n  mode: BandPass\n  frequency: 1000.0\n  q: 0.8\n"
            "- type: Delay\n  time: 0.25\n  mix: 0.25";
        if (presetId == 7) return
            "# Parallel Processing with a Loop\n"
            "- type: Group\n"
            "  mode: Parallel\n"
            "  children:\n"
            "    - type: Filter\n"
            "      mode: LowPass\n"
            "      frequency: 400.0\n"
            "    - type: Group\n"
            "      mode: Series\n"
            "      repeat: 3\n"
            "      children:\n"
            "        - type: Distortion\n"
            "          drive: 2.0";
            
        // Default
        return "# Paste this directly into the plugin\n"
               "- type: Gain\n"
               "  gain_db:\n"
               "    value: -6.0\n"
               "    ui: Slider\n"
               "    style: Rotary\n"
               "\n"
               "- type: Filter\n"
               "  mode: LowPass\n"
               "  frequency: 1000.0";
    }
    else if (languageId == 4) // Python
    {
        juce::String header = "from preset_engine import Chain, Gain, Filter, Compressor, Limiter, Delay, Reverb, Distortion, NoiseGate, Phaser\nchain = Chain()\n";
        juce::String footer = "print(chain.to_yaml())";
        
        if (presetId == 2) return header + 
            "chain.add(NoiseGate(threshold=-60.0, ratio=10.0))\n"
            "chain.add(Filter(mode='HighPass', frequency=80.0))\n"
            "chain.add(Compressor(threshold=-20.0, ratio=4.0, attack=5.0, release=100.0))\n" + footer;
        if (presetId == 3) return header +
            "chain.add(Distortion(drive=6.0))\n"
            "chain.add(Compressor(threshold=-15.0, ratio=8.0, attack=1.0, release=50.0))\n"
            "chain.add(Gain(db=3.0))\n" + footer;
        if (presetId == 4) return header +
            "chain.add(Delay(time=0.4, feedback=0.6, mix=0.4))\n"
            "chain.add(Reverb(room_size=0.8, wet=0.5))\n"
            "chain.add(Phaser(rate=0.2, depth=0.7))\n" + footer;
        if (presetId == 5) return header +
            "chain.add(Filter(mode='HighPass', frequency=30.0))\n"
            "chain.add(Compressor(threshold=-10.0, ratio=2.0, attack=30.0, release=200.0))\n"
            "chain.add(Limiter(threshold=-0.1, release=100.0))\n" + footer;
        if (presetId == 6) return header +
            "chain.add(Distortion(drive=12.0))\n"
            "chain.add(Filter(mode='BandPass', frequency=1000.0, q=0.8))\n"
            "chain.add(Delay(time=0.25, mix=0.25))\n" + footer;
        if (presetId == 7) return 
            "# Nested groups are not yet supported in the simple Python transpiler.\n"
            "# Please use YAML for complex structures.\n"
            "print('''\n"
            "- type: Group\n"
            "  mode: Parallel\n"
            "  children:\n"
            "    - type: Filter\n"
            "      mode: LowPass\n"
            "      frequency: 400.0\n"
            "    - type: Group\n"
            "      mode: Series\n"
            "      repeat: 3\n"
            "      children:\n"
            "        - type: Distortion\n"
            "          drive: 2.0\n"
            "''')";

        return "# SDK USAGE - Run this Python script to GENERATE a preset file\n"
               "from preset_engine import Chain, Gain\n"
               "chain = Chain()\n"
               "chain.add(Gain(db=-6.0))\n"
               "print(chain.to_yaml())";
    }
    
    // Fallback for JSON/XML (Just minimal support for now)
    return "Not implemented for this language yet.";
}

juce::String PresetEngineAudioProcessorEditor::parsePythonToYaml(const juce::String& pythonCode)
{
    juce::String yaml = "";
    juce::StringArray lines;
    lines.addLines(pythonCode);

    for (auto& line : lines)
    {
        line = line.trim();
        if (line.startsWith("chain.add("))
        {
            // Extract content inside chain.add(...)
            int start = line.indexOf("(") + 1;
            int end = line.lastIndexOf(")");
            
            if (end > start)
            {
                juce::String content = line.substring(start, end).trim();
                
                // Parse "EffectName(args)"
                int parenOpen = content.indexOf("(");
                int parenClose = content.lastIndexOf(")");
                
                if (parenOpen > 0 && parenClose > parenOpen)
                {
                    juce::String type = content.substring(0, parenOpen).trim();
                    juce::String args = content.substring(parenOpen + 1, parenClose);
                    
                    yaml += "- type: " + type + "\n";
                    
                    // Parse args: key=value, key=value
                    juce::StringArray parts;
                    parts.addTokens(args, ",", "\"'"); 
                    
                    for (auto& part : parts)
                    {
                        part = part.trim();
                        int eq = part.indexOf("=");
                        if (eq > 0)
                        {
                            juce::String key = part.substring(0, eq).trim();
                            juce::String val = part.substring(eq + 1).trim();
                            
                            // Remove quotes if present
                            val = val.unquoted();
                            
                            yaml += "  " + key + ": " + val + "\n";
                        }
                    }
                    yaml += "\n";
                }
            }
        }
    }
    return yaml;
}

void PresetEngineAudioProcessorEditor::rebuildUi()
{
    container->removeAllChildren();
    effectComponents.clear();

    auto tree = audioProcessor.getCurrentConfigTree();
    
    // Recursive helper to flatten the tree with indentation
    std::function<void(const juce::ValueTree&, int)> addEffects;
    addEffects = [&](const juce::ValueTree& parent, int level)
    {
        for (const auto& child : parent)
        {
            // Check if it is an effect (has type)
            if (child.hasProperty("type"))
            {
                auto* comp = new DynamicEffectComponent(child);
                comp->setIndent(level);
                effectComponents.add(comp);
                container->addAndMakeVisible(comp);
                
                // Recurse for children
                addEffects(child, level + 1);
            }
        }
    };

    // Start recursion from root
    addEffects(tree, 0);
    
    // Layout logic: Stack effects vertically
    int y = 0;
    const int effectHeight = 110;
    
    int w = container->getWidth();
    if (w <= 0) w = viewport.getWidth() - 20; // Fallback to viewport width
    if (w <= 0) w = 400; // Fallback to default

    for (auto* comp : effectComponents)
    {
        comp->setBounds(0, y, w, effectHeight);
        y += effectHeight + 5;
    }

    if (container->getWidth() > 0)
    {
         // Refresh bounds
         for (auto* comp : effectComponents)
            comp->setBounds(0, comp->getY(), container->getWidth(), comp->getHeight());
    }

    container->setSize(w, std::max(300, y));
}

void PresetEngineAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Modern Dark Background
    g.fillAll (juce::Colour(0xff121212));

    // Header background (Top strip)
    g.setColour(juce::Colour(0xff1e1e1e));
    g.fillRect(0, 0, getWidth(), 40);

    // Separator
    g.setColour(juce::Colours::black);
    g.drawHorizontalLine(40, 0.0f, (float)getWidth());
}

void PresetEngineAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Header
    auto header = area.removeFromTop(40);
    titleLabel.setBounds(header.removeFromTop(22).reduced(10, 0));
    subtitleLabel.setBounds(header.reduced(10, 0));

    // Split: Left (40%), Right (60%)
    auto leftArea = area.removeFromLeft((int)(getWidth() * 0.4));
    auto rightArea = area; // Remaining

    // Right Side: Viewport (Controls)
    viewport.setBounds(rightArea.reduced(10));
    if (container)
        container->setSize(viewport.getWidth() - 15, container->getHeight());

    // Left Side:
    // Top 33%: Spectrum
    auto spectrumArea = leftArea.removeFromTop((int)(leftArea.getHeight() * 0.33));
    spectrumComponent.setBounds(spectrumArea.reduced(10));
    
    // Middle: Controls strip
    auto controlsArea = leftArea.removeFromTop(40);
    controlsArea.reduce(10, 5);
    
    languageBox.setBounds(controlsArea.removeFromLeft(70));
    controlsArea.removeFromLeft(5);
    presetBox.setBounds(controlsArea.removeFromLeft(120));
    controlsArea.removeFromLeft(5);
    exampleButton.setBounds(controlsArea.removeFromLeft(60));
    controlsArea.removeFromLeft(5);
    applyButton.setBounds(controlsArea.removeFromRight(70));
    statusLabel.setBounds(controlsArea);
    
    // Bottom: Code Editor
    codeEditor.setBounds(leftArea.reduced(10));
    
    // Trigger rebuild layout to ensure container widths are correct
    if (container && container->getWidth() > 0)
    {
        int y = 0;
        const int effectHeight = 110;
        for (auto* comp : effectComponents)
        {
            comp->setBounds(0, y, container->getWidth(), effectHeight);
            y += effectHeight + 5;
        }
        container->setSize(container->getWidth(), std::max(300, y));
    }
}
