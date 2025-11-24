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
            label.setText(name.toUpperCase(), juce::dontSendNotification); // All caps for modern look
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
        // Title (We can make this invisible or very subtle for minimalism)
        juce::String type = effectTree.getProperty("type").toString();
        // group.setText(type); // Don't show group text frame, just a label maybe?
        // Actually let's just use the FlexBox to flow items. Maybe add a Label for the block name.

        effectNameLabel.setText(type.toUpperCase(), juce::dontSendNotification);
        effectNameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        effectNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
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
            simpleConfig.setProperty("ui", "Slider", nullptr);

            auto* comp = new DynamicParameterComponent(name, simpleConfig);
            params.add(comp);
            addAndMakeVisible(comp);
        }

        // 2. Children (Complex params)
        for (const auto& child : effectTree)
        {
            auto name = child.getType().toString();
            auto* comp = new DynamicParameterComponent(name, child);
            params.add(comp);
            addAndMakeVisible(comp);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        
        // Header
        effectNameLabel.setBounds(area.removeFromTop(25).reduced(5, 0));

        // Content
        juce::FlexBox flex;
        flex.flexWrap = juce::FlexBox::Wrap::wrap;
        flex.justifyContent = juce::FlexBox::JustifyContent::flexStart;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        for (auto* p : params)
        {
            flex.items.add(juce::FlexItem(*p).withWidth(70.0f).withHeight(80.0f).withMargin(5.0f));
        }

        flex.performLayout(area);
    }

private:
    juce::Label effectNameLabel;
    juce::OwnedArray<DynamicParameterComponent> params;
};

//==============================================================================
PresetEngineAudioProcessorEditor::PresetEngineAudioProcessorEditor (PresetEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Apply custom look and feel
    setLookAndFeel(&lookAndFeel);

    setSize (900, 500); // Widescreen modern aspect

    // Header
    titleLabel.setText("PRESET ENGINE 2026", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00bcd4)); // Cyan title
    addAndMakeVisible(titleLabel);

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
    addAndMakeVisible(languageBox);

    exampleButton.setButtonText("Load Example");
    exampleButton.onClick = [this] {
        int id = languageBox.getSelectedId();
        juce::String example = "";
        if (id == 1) // YAML
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
        else if (id == 2) // JSON
        {
             example = "[\n"
                     "  {\n"
                     "    \"type\": \"Gain\",\n"
                     "    \"gain_db\": {\n"
                     "      \"value\": -6.0,\n"
                     "      \"ui\": \"Slider\"\n"
                     "    }\n"
                     "  }\n"
                     "]";
        }
        else if (id == 3) // XML
        {
             example = "<EffectChain>\n"
                     "  <Effect type=\"Gain\">\n"
                     "    <gain_db value=\"-6.0\" ui=\"Slider\"/>\n"
                     "  </Effect>\n"
                     "</EffectChain>";
        }
        else if (id == 4) // Python
        {
             example = "# SDK USAGE - Run this Python script to GENERATE a preset file\n"
                     "from preset_engine import Chain, Gain\n"
                     "chain = Chain()\n"
                     "chain.add(Gain(db=-6.0))\n"
                     "print(chain.to_yaml())";
        }
        codeEditor.setText(example);
    };
    addAndMakeVisible(exampleButton);

    codeEditor.setMultiLine(true);
    codeEditor.setReturnKeyStartsNewLine(true);
    codeEditor.setTabKeyUsedAsCharacter(true);
    codeEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain));
    
    if (audioProcessor.getCurrentConfig().isEmpty())
        exampleButton.triggerClick();
    else
        codeEditor.setText(audioProcessor.getCurrentConfig());
        
    addAndMakeVisible(codeEditor);

    statusLabel.setText("Ready.", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(12.0f));
    statusLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(statusLabel);

    applyButton.setButtonText("APPLY");
    applyButton.onClick = [this] {
        auto result = audioProcessor.loadConfig(codeEditor.getText());
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

void PresetEngineAudioProcessorEditor::rebuildUi()
{
    container->removeAllChildren();
    effectComponents.clear();

    auto tree = audioProcessor.getCurrentConfigTree();
    
    // Layout logic: Stack effects vertically
    int y = 0;
    const int effectHeight = 110;

    for (const auto& child : tree)
    {
        auto* comp = new DynamicEffectComponent(child);
        effectComponents.add(comp);
        container->addAndMakeVisible(comp);
        
        comp->setBounds(0, y, container->getWidth(), effectHeight);
        y += effectHeight + 5;
    }

    if (container->getWidth() > 0)
    {
         // Refresh bounds
         for (auto* comp : effectComponents)
            comp->setBounds(0, comp->getY(), container->getWidth(), comp->getHeight());
    }

    container->setSize(container->getWidth(), std::max(300, y));
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
    titleLabel.setBounds(header.reduced(10, 0));

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
    
    languageBox.setBounds(controlsArea.removeFromLeft(80));
    controlsArea.removeFromLeft(5);
    exampleButton.setBounds(controlsArea.removeFromLeft(100));
    controlsArea.removeFromLeft(5);
    applyButton.setBounds(controlsArea.removeFromRight(80));
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
