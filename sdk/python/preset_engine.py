import json
import yaml

class Effect:
    def __init__(self, type_name, **kwargs):
        self.type = type_name
        self.params = kwargs

    def to_dict(self):
        d = {"type": self.type}
        for k, v in self.params.items():
            if isinstance(v, Parameter):
                d[k] = v.to_dict()
            else:
                d[k] = v
        return d

class Parameter:
    def __init__(self, value, ui=None, style=None, min_val=None, max_val=None, options=None):
        self.value = value
        self.ui = ui
        self.style = style
        self.min = min_val
        self.max = max_val
        self.options = options

    def to_dict(self):
        d = {"value": self.value}
        if self.ui: d["ui"] = self.ui
        if self.style: d["style"] = self.style
        if self.min is not None: d["min"] = self.min
        if self.max is not None: d["max"] = self.max
        if self.options: d["options"] = self.options
        return d

class Chain:
    def __init__(self):
        self.effects = []

    def add(self, effect):
        self.effects.append(effect)
        return self

    def to_json(self):
        return json.dumps([e.to_dict() for e in self.effects], indent=2)

    def to_yaml(self):
        return yaml.dump([e.to_dict() for e in self.effects], sort_keys=False)

# --- Effect Wrappers ---

def Gain(db=0.0, ui=False):
    if ui:
        return Effect("Gain", gain_db=Parameter(db, ui="Slider", style="Linear", min=-60, max=12))
    return Effect("Gain", gain_db=db)

def Filter(mode="LowPass", freq=1000.0, q=0.7, ui=False):
    if ui:
        return Effect("Filter", 
                      type=Parameter(mode, ui="ComboBox", options=["LowPass", "HighPass", "BandPass"]),
                      frequency=Parameter(freq, ui="Slider", style="Rotary", min=20, max=20000),
                      q=Parameter(q, ui="Slider", min=0.1, max=10))
    return Effect("Filter", type=mode, frequency=freq, q=q)

def Reverb(size=0.5, wet=0.33):
    return Effect("Reverb", room_size=size, wet=wet)
