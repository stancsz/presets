package main

import (
	"encoding/json"
	"fmt"
	"os"
)

// Parameter represents a complex UI-aware parameter
type Parameter struct {
	Value   interface{} `json:"value"`
	UI      string      `json:"ui,omitempty"`
	Style   string      `json:"style,omitempty"`
	Min     float64     `json:"min,omitempty"`
	Max     float64     `json:"max,omitempty"`
	Options []string    `json:"options,omitempty"`
}

// Effect is a generic map to hold any effect configuration
type Effect map[string]interface{}

// Chain is a list of effects
type Chain []Effect

// NewChain creates a new empty chain
func NewChain() *Chain {
	return &Chain{}
}

// Add appends an effect to the chain
func (c *Chain) Add(e Effect) {
	*c = append(*c, e)
}

// ToJSON returns the JSON string representation
func (c *Chain) ToJSON() (string, error) {
	b, err := json.MarshalIndent(c, "", "  ")
	return string(b), err
}

// --- Helpers ---

func Gain(db float64) Effect {
	return Effect{
		"type":    "Gain",
		"gain_db": db,
	}
}

func GainUI(db float64) Effect {
	return Effect{
		"type": "Gain",
		"gain_db": Parameter{
			Value: db,
			UI:    "Slider",
			Style: "LinearVertical",
			Min:   -60,
			Max:   12,
		},
	}
}

func Filter(mode string, freq float64) Effect {
	return Effect{
		"type":      "Filter",
		"type_mode": mode, // Note: 'type' is reserved for effect type, so we might need to handle this key collision in Go maps carefully or use a struct.
		// Actually, for the plugin 'type' property inside Filter, we can just use "mode" or "filter_type" if we updated the C++ side, 
		// but the C++ side expects "type" property for the filter mode too?
		// Let's check FilterEffect.h. It looks for "type". 
		// In JSON: { "type": "Filter", "type": "LowPass" } is invalid JSON (duplicate key).
		// Wait, my previous JSON example had this issue and I fixed it by changing the inner one to "mode".
		// I need to ensure C++ FilterEffect supports "mode".
		"frequency": freq,
	}
}

// We need to verify FilterEffect.h supports "mode". 
// I recall adding "mode" support for LadderFilter, but standard FilterEffect might still be looking for "type".
// I should check FilterEffect.h and update it if necessary to support "mode" to avoid JSON key collision.
