from PIL import Image, ImageDraw, ImageFont

# Canvas size
WIDTH, HEIGHT = 600, 500

# Colors (from ModernLookAndFeel.h)
BG_COLOR = (30, 30, 30) # 0xff1e1e1e
EDITOR_BG_COLOR = (37, 37, 38) # 0xff252526
EDITOR_TEXT_COLOR = (212, 212, 212) # 0xffd4d4d4
BUTTON_COLOR = (14, 99, 156) # 0xff0e639c
BUTTON_TEXT_COLOR = (255, 255, 255)
STATUS_TEXT_COLOR = (128, 128, 128) # Grey
PANEL_BG_COLOR = (45, 45, 48) # 0xff2d2d30
PANEL_BORDER_COLOR = (0, 0, 0, 76) # Alpha 0.3

# Fonts
try:
    font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20)
    font_code = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14)
    font_button = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 15)
    font_status = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
    font_label_bold = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14)
    font_label_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 11)
except IOError:
    font_title = ImageFont.load_default()
    font_code = ImageFont.load_default()
    font_button = ImageFont.load_default()
    font_status = ImageFont.load_default()
    font_label_bold = ImageFont.load_default()
    font_label_small = ImageFont.load_default()

def draw_common_ui(draw, title_text, btn_text):
    # Header
    draw.text((20, 30), "Yaml Preset Plugin", font=font_title, fill=(255, 255, 255))

    # Toggle Button (Top Right)
    btn_w, btn_h = 180, 30
    btn_x, btn_y = WIDTH - 20 - btn_w, 25
    draw.rounded_rectangle((btn_x, btn_y, btn_x + btn_w, btn_y + btn_h), 6, fill=BUTTON_COLOR)
    draw.text((btn_x + 15, btn_y + 7), btn_text, font=font_button, fill=BUTTON_TEXT_COLOR)

    # Footer
    draw.text((20, HEIGHT - 30), "Ready.", font=font_status, fill=STATUS_TEXT_COLOR)

def generate_code_view():
    image = Image.new("RGB", (WIDTH, HEIGHT), BG_COLOR)
    draw = ImageDraw.Draw(image)

    draw_common_ui(draw, "Yaml Preset Plugin", "Switch to Visual View")

    # Editor Area
    area_x, area_y = 20, 70
    area_w, area_h = WIDTH - 40, HEIGHT - 120

    draw.rectangle((area_x, area_y, area_x + area_w, area_y + area_h), fill=EDITOR_BG_COLOR)

    code_text = """- type: Gain
  gain: 0.8
- type: Drive
  drive: 0.4
- type: Delay
  time: 350.0
  feedback: 0.4
  mix: 0.3
- type: Reverb
  room_size: 0.7
  wet: 0.4
"""
    text_x = area_x + 10
    text_y = area_y + 10
    line_height = 18
    for i, line in enumerate(code_text.split('\n')):
        draw.text((text_x, text_y + i * line_height), line, font=font_code, fill=EDITOR_TEXT_COLOR)

    # Apply Button
    btn_w, btn_h = 150, 30
    btn_x = WIDTH - 20 - btn_w
    btn_y = HEIGHT - 40
    draw.rounded_rectangle((btn_x, btn_y, btn_x + btn_w, btn_y + btn_h), 6, fill=BUTTON_COLOR)
    draw.text((btn_x + 10, btn_y + 7), "Apply Configuration", font=font_button, fill=BUTTON_TEXT_COLOR)

    image.save("screenshot_code.png")
    print("Generated screenshot_code.png")

def draw_knob(draw, x, y, label):
    # Simple circle knob representation
    r = 20
    cx, cy = x + 30, y + 30
    draw.ellipse((cx - r, cy - r, cx + r, cy + r), outline=(180, 180, 180), width=2)
    # Indicator line
    draw.line((cx, cy, cx + 10, cy - 10), fill=(200, 200, 200), width=2)

    # Label
    draw.text((x + 5, y + 55), label, font=font_label_small, fill=(200, 200, 200))

def generate_visual_view():
    image = Image.new("RGB", (WIDTH, HEIGHT), BG_COLOR)
    draw = ImageDraw.Draw(image, "RGBA")

    draw_common_ui(draw, "Yaml Preset Plugin", "Switch to Code View")

    area_x, area_y = 20, 70
    panel_h = 80
    spacing = 5

    effects = [
        ("Gain", ["Gain"]),
        ("Overdrive", ["Drive"]),
        ("Delay", ["Time (ms)", "Feedback", "Mix"]),
        ("Reverb", ["Room Size", "Damping", "Wet", "Dry", "Width"])
    ]

    current_y = area_y

    for name, params in effects:
        # Panel BG
        draw.rounded_rectangle((area_x, current_y, WIDTH - 20, current_y + panel_h), 6, fill=PANEL_BG_COLOR)
        draw.rounded_rectangle((area_x, current_y, WIDTH - 20, current_y + panel_h), 6, outline=PANEL_BORDER_COLOR, width=1)

        # Name
        draw.text((area_x + 10, current_y + 30), name, font=font_label_bold, fill=(220, 220, 220))

        # Knobs
        knob_start_x = area_x + 120
        knob_spacing = 70
        for i, param in enumerate(params):
            draw_knob(draw, knob_start_x + i * knob_spacing, current_y, param)

        current_y += panel_h + spacing

    image.save("screenshot_visual.png")
    print("Generated screenshot_visual.png")

if __name__ == "__main__":
    generate_code_view()
    generate_visual_view()
