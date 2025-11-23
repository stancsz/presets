import sys
import os

# Add sdk path
sys.path.append(os.path.join(os.path.dirname(__file__), '../sdk/python'))

from preset_engine import Chain, Gain, Filter, Reverb

def main():
    # Create a chain
    chain = Chain()
    
    # Add effects
    chain.add(Gain(db=-6.0, ui=True))
    chain.add(Filter(mode="BandPass", freq=500.0, q=2.0, ui=True))
    chain.add(Reverb(size=0.8, wet=0.5))

    # Output YAML
    print("--- YAML Output ---")
    print(chain.to_yaml())

    # Output JSON
    print("\n--- JSON Output ---")
    print(chain.to_json())

    # Save to file
    with open("generated_preset.yaml", "w") as f:
        f.write(chain.to_yaml())
    print("\nSaved to generated_preset.yaml")

if __name__ == "__main__":
    main()
