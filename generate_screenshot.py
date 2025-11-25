from PIL import Image, ImageDraw, ImageFont
import math

# Constants matching C++ Code
WIDTH = 900
HEIGHT = 500

COLOR_BG = "#121212"
COLOR_HEADER_BG = "#1e1e1e"
COLOR_ACCENT = "#00bcd4"
COLOR_TEXT_MAIN = "#e0e0e0"
COLOR_TEXT_DIM = "#888888"
COLOR_EDITOR_BG = "#1e1e1e"
COLOR_SLIDER_TRACK = "#2d2d2d"

# Fonts (approximate since we don't have the exact system fonts)
# We'll use default load_default() but try to find a ttf if possible, otherwise fallback.
try:
    font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24)
    font_label = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 10)
    font_code = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 12)
    font_std = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12)
except IOError:
    font_title = ImageFont.load_default()
    font_label = ImageFont.load_default()
    font_code = ImageFont.load_default()
    font_std = ImageFont.load_default()

def draw_rotary(draw, x, y, size, value, name):
    # Center
    cx = x + size // 2
    cy = y + size // 2
    radius = (size // 2) - 4

    # Angles (approx Juce default rotary)
    start_angle = 135 + 90
    end_angle = 405 + 90
    current_angle = start_angle + value * (end_angle - start_angle)

    # Background Arc
    draw.arc([cx-radius, cy-radius, cx+radius, cy+radius], start=start_angle, end=end_angle, fill=COLOR_SLIDER_TRACK, width=4)

    # Active Arc
    draw.arc([cx-radius, cy-radius, cx+radius, cy+radius], start=start_angle, end=current_angle, fill=COLOR_ACCENT, width=4)

    # Label
    w = draw.textlength(name, font=font_label)
    draw.text((cx - w/2, y + size), name, fill=COLOR_TEXT_DIM, font=font_label)

def draw_linear(draw, x, y, w, h, value, name):
    # Vertical Fader
    # Track
    track_w = 4
    track_x = x + (w - track_w) // 2
    draw.rectangle([track_x, y, track_x + track_w, y + h], fill=COLOR_SLIDER_TRACK)

    # Fill
    fill_h = h * value
    draw.rectangle([track_x, y + h - fill_h, track_x + track_w, y + h], fill=COLOR_ACCENT)

    # Handle
    handle_w = 16
    handle_h = 8
    handle_y = y + h - fill_h - (handle_h // 2)
    handle_x = x + (w - handle_w) // 2
    draw.rectangle([handle_x, handle_y, handle_x + handle_w, handle_y + handle_h], fill="white")

    # Label
    text_w = draw.textlength(name, font=font_label)
    draw.text((x + (w - text_w)/2, y + h + 5), name, fill=COLOR_TEXT_DIM, font=font_label)

def generate_screenshot():
    img = Image.new("RGB", (WIDTH, HEIGHT), COLOR_BG)
    draw = ImageDraw.Draw(img)

    # --- Header ---
    draw.rectangle([0, 0, WIDTH, 40], fill=COLOR_HEADER_BG)
    draw.line([0, 40, WIDTH, 40], fill="black")
    draw.text((10, 5), "PRESET ENGINE 2026", fill=COLOR_ACCENT, font=font_title)

    # --- Layout Areas ---
    # Left: 0 to 360 (40%)
    # Right: 360 to 900 (60%)

    # Left Column Content
    # 1. Spectrum (Top 33% of remaining 460px = ~150px)
    spectrum_h = 150
    spectrum_y = 50 # 40 + padding 10
    spectrum_w = 340 # 360 - 20 padding
    spectrum_x = 10

    # Draw Spectrum BG
    draw.rectangle([spectrum_x, spectrum_y, spectrum_x + spectrum_w, spectrum_y + spectrum_h], fill="#000000")

    # Draw Gradient Curve (Mock)
    points = []
    for i in range(spectrum_w):
        x_norm = i / spectrum_w
        # Simulate sum of sines
        y_norm = 0.3 * math.sin(x_norm * 10) + 0.2 * math.sin(x_norm * 23) + 0.1 * math.sin(x_norm * 50)
        y_norm = abs(y_norm) * math.exp(-2 * x_norm)

        y = spectrum_h - (y_norm + 0.1) * spectrum_h * 2.0 # Scale
        y = max(0, min(spectrum_h, y))
        points.append((spectrum_x + i, spectrum_y + y))

    # Fill under curve
    # To do a gradient fill in PIL is complex, we'll just do solid or lines
    # Let's do vertical lines for gradient effect
    for px, py in points:
        draw.line([px, py, px, spectrum_y + spectrum_h], fill=COLOR_ACCENT)
        # Hacky gradient: draw black over it with decreasing alpha?
        # Easier: Just draw the line cyan.

    # 2. Controls Strip
    controls_y = spectrum_y + spectrum_h + 10
    # Dropdown
    draw.rectangle([20, controls_y, 100, controls_y + 25], fill="#2d2d2d", outline="#555")
    draw.text((30, controls_y+5), "YAML", fill="white", font=font_std)
    # Button
    draw.rectangle([110, controls_y, 210, controls_y + 25], fill="#2d2d2d", outline="#555")
    draw.text((120, controls_y+5), "Load Example", fill="white", font=font_std)
    # Apply
    draw.rectangle([270, controls_y, 340, controls_y + 25], fill="#2d2d2d", outline="#555")
    draw.text((285, controls_y+5), "APPLY", fill="white", font=font_std)

    # 3. Code Editor
    editor_y = controls_y + 35
    editor_h = HEIGHT - editor_y - 10
    draw.rectangle([10, editor_y, 350, editor_y + editor_h], fill=COLOR_EDITOR_BG)
    code_text = """- type: Gain
  gain_db:
    value: -6.0
    ui: Slider
    style: Rotary

- type: Filter
  mode: LowPass
  frequency: 1000.0"""
    draw.text((20, editor_y + 10), code_text, fill=COLOR_TEXT_MAIN, font=font_code)

    # --- Right Column ---
    right_x = 360
    right_w = WIDTH - right_x

    # Effect 1: Gain
    eff1_y = 50
    draw.text((right_x + 10, eff1_y), "GAIN", fill="white", font=font_std)
    # Param 1
    draw_rotary(draw, right_x + 20, eff1_y + 20, 60, 0.4, "GAIN DB")

    # Effect 2: Filter
    eff2_y = eff1_y + 110
    draw.text((right_x + 10, eff2_y), "FILTER", fill="white", font=font_std)
    draw_rotary(draw, right_x + 20, eff2_y + 20, 60, 0.7, "FREQ")
    draw_rotary(draw, right_x + 100, eff2_y + 20, 60, 0.2, "RES")

    # Effect 3: ADSR (Example of knobs)
    eff3_y = eff2_y + 110
    draw.text((right_x + 10, eff3_y), "ENVELOPE", fill="white", font=font_std)
    draw_rotary(draw, right_x + 20, eff3_y + 20, 60, 0.1, "ATTACK")
    draw_rotary(draw, right_x + 100, eff3_y + 20, 60, 0.3, "DECAY")
    draw_rotary(draw, right_x + 180, eff3_y + 20, 60, 0.8, "SUSTAIN")
    draw_rotary(draw, right_x + 260, eff3_y + 20, 60, 0.4, "RELEASE")

    # Effect 4: Mixer (Linear Sliders)
    eff4_y = eff3_y + 110
    draw.text((right_x + 10, eff4_y), "MIXER", fill="white", font=font_std)
    draw_linear(draw, right_x + 20, eff4_y + 20, 40, 80, 0.8, "LFO 1")
    draw_linear(draw, right_x + 80, eff4_y + 20, 40, 80, 0.6, "LFO 2")
    draw_linear(draw, right_x + 140, eff4_y + 20, 40, 80, 0.9, "MASTER")

    img.save("screenshot.png")
    print("Screenshot generated: screenshot.png")

if __name__ == "__main__":
    generate_screenshot()
