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

# Create image
image = Image.new("RGB", (WIDTH, HEIGHT), BG_COLOR)
draw = ImageDraw.Draw(image)

# Fonts (using basic defaults as we might not have custom fonts)
try:
    # Try to load a nice font if available, else default
    font_title = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20)
    font_code = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14)
    font_button = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 15)
    font_status = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
except IOError:
    font_title = ImageFont.load_default()
    font_code = ImageFont.load_default()
    font_button = ImageFont.load_default()
    font_status = ImageFont.load_default()

# --- Layout (PluginEditor.cpp) ---

# Margins: reduced(20)
area_x, area_y = 20, 20
area_w, area_h = WIDTH - 40, HEIGHT - 40

# Header
header_h = 40
draw.text((area_x, area_y + 10), "Yaml Preset Plugin", font=font_title, fill=(255, 255, 255))

# Footer
footer_h = 40
footer_y = area_y + area_h - footer_h

# Button (Bottom Right)
btn_w = 150
btn_h = 30
btn_x = area_x + area_w - btn_w
btn_y = footer_y + (footer_h - btn_h) // 2

# Draw Button Rounded Rect
def draw_rounded_rect(draw, box, radius, fill):
    x, y, w, h = box
    draw.rounded_rectangle((x, y, x + w, y + h), radius, fill=fill)

draw_rounded_rect(draw, (btn_x, btn_y, btn_w, btn_h), 6, BUTTON_COLOR)
draw.text((btn_x + 15, btn_y + 7), "Apply Configuration", font=font_button, fill=BUTTON_TEXT_COLOR)

# Status (Bottom Left)
status_x = area_x
status_y = footer_y + (footer_h - btn_h) // 2 + 5
draw.text((status_x, status_y), "Ready.", font=font_status, fill=STATUS_TEXT_COLOR)

# Editor Area
editor_y = area_y + header_h + 10
editor_h = footer_y - 10 - editor_y
editor_w = area_w

# Draw Editor Background
draw.rectangle((area_x, editor_y, area_x + editor_w, editor_y + editor_h), fill=EDITOR_BG_COLOR)

# Draw Code Text
code_text = """- type: Gain
  gain: 0.5
- type: Filter
  type: LowPass
  frequency: 1000
  q: 0.707
- type: Compressor
  threshold: -10
  ratio: 4.0
  attack: 5.0
  release: 100.0
- type: Limiter
  threshold: -0.1
  release: 50.0
"""

text_x = area_x + 10
text_y = editor_y + 10
line_height = 18

for i, line in enumerate(code_text.split('\n')):
    draw.text((text_x, text_y + i * line_height), line, font=font_code, fill=EDITOR_TEXT_COLOR)

# Save
image.save("screenshot.png")
print("Screenshot generated: screenshot.png")
