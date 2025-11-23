package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
)

// Import the SDK (simulated here as it's in a local folder, normally would be a module)
// For this example, we'll just copy the struct definitions or assume they are in the same package if we run 'go run *.go'
// But to be proper, let's assume the user copies the sdk code or we just inline the usage for this example script.
// Since I can't easily set up a go.mod workspace here, I will just replicate the logic to demonstrate the "User Experience".

type Parameter struct {
	Value   interface{} `json:"value"`
	UI      string      `json:"ui,omitempty"`
	Style   string      `json:"style,omitempty"`
	Min     float64     `json:"min,omitempty"`
	Max     float64     `json:"max,omitempty"`
}

type Effect map[string]interface{}

func main() {
	chain := []Effect{}

	// Add Gain
	chain = append(chain, Effect{
		"type": "Gain",
		"gain_db": Parameter{
			Value: -6.0,
			UI:    "Slider",
			Style: "Linear",
			Min:   -60,
			Max:   0,
		},
	})

	// Add Filter
	chain = append(chain, Effect{
		"type":      "Filter",
		"mode":      "HighPass",
		"frequency": 2000.0,
	})

	// Add Distortion
	chain = append(chain, Effect{
		"type": "Distortion",
		"drive": Parameter{
			Value: 20.0,
			UI:    "Slider",
			Style: "Rotary",
		},
	})

	// Generate JSON
	b, _ := json.MarshalIndent(chain, "", "  ")
	
	fmt.Println("--- JSON Output ---")
	fmt.Println(string(b))

	// Save
	os.WriteFile("generated_preset.json", b, 0644)
	fmt.Println("\nSaved to generated_preset.json")
}
