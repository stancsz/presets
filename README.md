# PresetEngine

**A Declarative, Text-Driven Modular Multi-Effect Plugin.**

PresetEngine is a VST3 and Standalone audio plugin that allows you to define complex audio processing chains and custom user interfaces entirely through text configuration. It bridges the gap between DSP coding and rapid prototyping.

## What is it?

Think of it as a "browser" for audio processing chains. Instead of hardcoding a plugin with a fixed set of features, PresetEngine reads a configuration file (YAML, JSON, or XML) and dynamically constructs:
1.  **The DSP Chain**: Instantiating and connecting audio effects in the specified order.
2.  **The User Interface**: Generating knobs, sliders, dropdowns, and meters to control those effects.

Technically, PresetEngine's configurable architecture makes it possible to recreate the vast majority of common commercial plugins (covering roughly 90% of typical use-cases) as well as support advanced, custom plugin behaviors — all defined via text configuration or programmatically.

## Key Features

*   **📄 Text-to-DSP**: Define your signal chain using simple, human-readable YAML (or JSON/XML).
*   **🎨 Dynamic UI Generation**: The plugin automatically builds a professional GUI based on your config. You define the controls (`ui: Slider`, `style: Rotary`), and the engine handles the layout.
*   **⚡ Hot-Reloading**: Edit the configuration directly inside the plugin window. Click "Apply" to instantly rebuild the DSP chain and UI without restarting your DAW.
*   **🔌 Format Agnostic**: Native support for **YAML**, **JSON**, and **JUCE XML**.
*   **🐍 Python & Go SDKs**: Generate complex presets programmatically using our provided SDKs. PresetEngine supports programmable languages such as Python and Go, enabling programmatic preset generation and tight integrations with external tooling.
*   **🎛️ Comprehensive DSP Library**: Built on the robust JUCE DSP module.

## Supported Effects

The plugin currently includes a wide range of studio-quality modules:

*   **Dynamics**: Compressor, Limiter, Noise Gate
*   **EQ & Filters**: IIR Filter (Low/High/BandPass), Ladder Filter (Moog-style drive)
*   **Spatial**: Reverb, Delay, Panner
*   **Modulation**: Chorus, Phaser
*   **Utility**: Gain, Distortion (WaveShaper)

## Example Configuration

Simply paste this into the plugin's editor to create a channel strip with a custom UI:

```yaml
- type: NoiseGate
  threshold: -60.0

- type: LadderFilter
  mode: LP24
  frequency:
    value: 400.0
    ui: Slider
    style: Rotary
  drive: 2.0

- type: Delay
  time: 0.375
  mix: 0.3
```

## Building

**Windows:**
Run the included build script:
```bat
build_release.bat
```
This will generate a `presets_windows.zip` in the `release/` folder containing the VST3 file.
