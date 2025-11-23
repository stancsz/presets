#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Component to render a single parameter
class DynamicParameterComponent : public juce::Component
{
public:
    DynamicParameterComponent(const juce::String& name, const juce::ValueTree& config)
        : paramName(name)
    {
        juce::String uiType = config.getProperty("ui", "Slider").toString();
        
        // Label (except for Button/Label types where it's redundant or handled differently)
        if (!uiType.equalsIgnoreCase("Button") && !uiType.equalsIgnoreCase("ToggleButton") && !uiType.equalsIgnoreCase("Label"))
        {
            addAndMakeVisible(label);
            label.setText(name, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.setFont(12.0f);
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
            
            s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
            
            float val = config.getProperty("value", 0.0f);
            float min = config.getProperty("min", 0.0f);
            float max = config.getProperty("max", 1.0f);
            
            if (!config.hasProperty("min") && !config.hasProperty("max"))
            {
                min = 0.0f; max = std::max(1.0f, std::abs(val) * 2.0f);
                if (max == 0) max = 1.0f;
            }

            s->setRange(min, max);
            s->setValue(val, juce::dontSendNotification);
        }
        else if (uiType.equalsIgnoreCase("Button") || uiType.equalsIgnoreCase("ToggleButton"))
        {
            auto* b = new juce::ToggleButton();
            component.reset(b);
            b->setButtonText(name);
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
                // JUCE ComboBox doesn't have a direct "setSelectedText", so we loop
                bool found = false;
                for (int i=0; i<c->getNumItems(); ++i)
                {
                    if (c->getItemText(i) == val.toString())
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
            component->setBounds(area.reduced(2));
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
class DynamicEffectComponent : public juce::Component
{
public:
    DynamicEffectComponent(const juce::ValueTree& effectTree)
    {
        // Title
        juce::String type = effectTree.getProperty("type").toString();
        group.setText(type);
        addAndMakeVisible(group);

        // Iterate properties and children to find parameters
        // 1. Properties (Simple scalars)
        for (int i = 0; i < effectTree.getNumProperties(); ++i)
        {
            auto name = effectTree.getPropertyName(i).toString();
            if (name == "type") continue;

            auto val = effectTree.getProperty(name);
            // Create a synthetic config tree for simple scalars
            juce::ValueTree simpleConfig("Param");
            simpleConfig.setProperty("value", val, nullptr);
            simpleConfig.setProperty("ui", "Slider", nullptr); // Default to slider

            auto* comp = new DynamicParameterComponent(name, simpleConfig);
            params.add(comp);
            addAndMakeVisible(comp);
        }

        // 2. Children (Complex params)
        for (const auto& child : effectTree)
        {
            auto name = child.getType().toString(); // Use node type as param name? Or name property?
            // In our schema: gain_db: { ... } -> Child name is "gain_db" (if we used property name as tag)
            // But yamlToValueTree uses the key as the tag name if it's a map.
            // So child.getType() is the parameter name.
            
            auto* comp = new DynamicParameterComponent(name, child);
            params.add(comp);
            addAndMakeVisible(comp);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        group.setBounds(area);
        
        auto content = area.reduced(10, 20); // Account for group title

        juce::FlexBox flex;
        flex.flexWrap = juce::FlexBox::Wrap::wrap;
        flex.justifyContent = juce::FlexBox::JustifyContent::flexStart;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        for (auto* p : params)
        {
            flex.items.add(juce::FlexItem(*p).withWidth(80.0f).withHeight(80.0f).withMargin(5.0f));
        }

        flex.performLayout(content);
    }

private:
    juce::GroupComponent group;
    juce::OwnedArray<DynamicParameterComponent> params;
};

//==============================================================================
PresetEngineAudioProcessorEditor::PresetEngineAudioProcessorEditor (PresetEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Apply custom look and feel
    setLookAndFeel(&lookAndFeel);

    setSize (800, 600);

    // Header
    titleLabel.setText("Preset Engine", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // Viewport for Dynamic UI
    addAndMakeVisible(viewport);
    viewport.setScrollBarsShown(true, false);
    
    // Container for effects
    container.reset(new juce::Component());
    viewport.setViewedComponent(container.get(), false);

    // Code Editor
    languageBox.addItem("YAML", 1);
    languageBox.addItem("JSON", 2);
    languageBox.addItem("XML", 3);
    languageBox.addItem("Python", 4);
    languageBox.addItem("Go", 5);
    languageBox.setSelectedId(1); // Default YAML
    addAndMakeVisible(languageBox);

    exampleButton.setButtonText("Load Example");
    exampleButton.onClick = [this] {
        int id = languageBox.getSelectedId();
        juce::String example = "";
        if (id == 1) // YAML - Direct Plugin Usage
        {
            example = "# Paste this directly into the plugin\n"
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
        else if (id == 2) // JSON - Direct Plugin Usage
        {
            example = "[\n"
                     "  {\n"
                     "    \"type\": \"Gain\",\n"
                     "    \"gain_db\": {\n"
                     "      \"value\": -6.0,\n"
                     "      \"ui\": \"Slider\"\n"
                     "    }\n"
                     "  },\n"
                     "  {\n"
                     "    \"type\": \"Filter\",\n"
                     "    \"mode\": \"LowPass\",\n"
                     "    \"frequency\": 1000.0\n"
                     "  }\n"
                     "]";
        }
        else if (id == 3) // XML - Direct Plugin Usage
        {
            example = "<EffectChain>\n"
                     "  <Effect type=\"Gain\">\n"
                     "    <gain_db value=\"-6.0\" ui=\"Slider\"/>\n"
                     "  </Effect>\n"
                     "  <Effect type=\"Filter\" mode=\"LowPass\" frequency=\"1000.0\"/>\n"
                     "</EffectChain>";
        }
        else if (id == 4) // Python - SDK Generator (NOT for direct paste)
        {
            example = "# SDK USAGE - Run this Python script to GENERATE a preset file\n"
                     "# Then load the generated .yaml/.json file into the plugin\n"
                     "#\n"
                     "# See: example/python_generator.py\n"
                     "\n"
                     "from preset_engine import Chain, Gain, Filter\n"
                     "\n"
                     "chain = Chain()\n"
                     "chain.add(Gain(db=-6.0, ui=True))\n"
                     "chain.add(Filter(mode=\"LowPass\", freq=1000.0))\n"
                     "\n"
                     "# Output YAML to paste into plugin:\n"
                     "print(chain.to_yaml())";
        }
        else if (id == 5) // Go - SDK Generator (NOT for direct paste)
        {
            example = "// SDK USAGE - Compile and run this Go program to GENERATE a preset\n"
                     "// Then load the generated .json file into the plugin\n"
                     "//\n"
                     "// See: example/go_generator.go\n"
                     "\n"
                     "package main\n"
                     "\n"
                     "import \"encoding/json\"\n"
                     "\n"
                     "func main() {\n"
                     "    chain := []Effect{\n"
                     "        {\"type\": \"Gain\", \"gain_db\": -6.0},\n"
                     "        {\"type\": \"Filter\", \"mode\": \"LowPass\"},\n"
                     "    }\n"
                     "    b, _ := json.MarshalIndent(chain, \"\", \"  \")\n"
                     "    fmt.Println(string(b))\n"
                     "}";
        }
        codeEditor.setText(example);
    };
    addAndMakeVisible(exampleButton);

    codeEditor.setMultiLine(true);
    codeEditor.setReturnKeyStartsNewLine(true);
    codeEditor.setTabKeyUsedAsCharacter(true);
    codeEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    
    // Only set text if empty (first run)
    if (audioProcessor.getCurrentConfig().isEmpty())
        exampleButton.triggerClick();
    else
        codeEditor.setText(audioProcessor.getCurrentConfig());
        
    addAndMakeVisible(codeEditor);

    // Status & Button
    statusLabel.setText("Ready.", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    applyButton.setButtonText("Apply");
    applyButton.onClick = [this] {
        // Pass the selected language ID as a hint? 
        // For now, we'll let the processor auto-detect, but we could prefix the code.
        // Actually, let's just pass the text. The parser will handle it.
        auto result = audioProcessor.loadConfig(codeEditor.getText());
        if (result.wasOk())
        {
            statusLabel.setText("Loaded.", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
            rebuildUi();
        }
        else
        {
            statusLabel.setText(result.getErrorMessage(), juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        }
    };
    addAndMakeVisible(applyButton);

    // Initial build
    rebuildUi();
}

PresetEngineAudioProcessorEditor::~PresetEngineAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    viewport.setViewedComponent(nullptr, false);
}

void PresetEngineAudioProcessorEditor::rebuildUi()
{
    container->removeAllChildren();
    effectComponents.clear();

    auto tree = audioProcessor.getCurrentConfigTree();
    
    // Layout logic: Stack effects vertically
    int y = 0;
    const int effectHeight = 120; // Fixed height for now, or dynamic

    for (const auto& child : tree)
    {
        auto* comp = new DynamicEffectComponent(child);
        effectComponents.add(comp);
        container->addAndMakeVisible(comp);
        
        comp->setBounds(0, y, 760, effectHeight); // Width matches viewport roughly
        y += effectHeight + 5;
    }

    container->setSize(760, std::max(300, y));
}

void PresetEngineAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PresetEngineAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // Header
    titleLabel.setBounds(area.removeFromTop(30));

    // Footer
    auto footer = area.removeFromBottom(180); // Increased for language selector
    
    // Top row: Language selector + Example button
    auto topRow = footer.removeFromTop(30);
    languageBox.setBounds(topRow.removeFromLeft(120));
    topRow.removeFromLeft(5);
    exampleButton.setBounds(topRow.removeFromLeft(120));
    
    footer.removeFromTop(5);
    
    // Second row: Apply button + Status
    auto buttonArea = footer.removeFromTop(30);
    applyButton.setBounds(buttonArea.removeFromRight(150));
    statusLabel.setBounds(buttonArea);

    footer.removeFromTop(5);
    codeEditor.setBounds(footer);

    // Main UI
    area.removeFromBottom(10);
    viewport.setBounds(area);
    
    if (container)
        container->setSize(viewport.getWidth() - 20, container->getHeight());
}
