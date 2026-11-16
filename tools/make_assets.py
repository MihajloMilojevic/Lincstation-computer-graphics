#!/usr/bin/env python3
"""Generiše asset-e: poluprovidni name overlay i 6 tematskih kursora
(novčanica, zelena novčanica, kovanica, katanac, ključ, ruka).

Upotreba:
  python3 tools/make_assets.py --name "Mihajlo" --surname "Milojević" --index "SV 00/2024"

Zahteva Pillow (pip install pillow). Font: DejaVu Sans (ili --font putanja do .ttf).
"""
import argparse, math, os
from PIL import Image, ImageDraw, ImageFont

OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "tex")
FONT_CANDIDATES = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/arial.ttf",
]

def find_font(arg):
    for p in ([arg] if arg else []) + FONT_CANDIDATES:
        if p and os.path.exists(p):
            return p
    raise SystemExit("Nije nađen .ttf font, prosledi --font")

def overlay(name, surname, index, font_path):
    # 240x44 u referentnim koordinatama, pravi se 2x (480x88)
    w, h = 480, 88
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((0, 0, w - 1, h - 1), radius=8, fill=(17, 17, 17, int(255 * 0.35)))
    f1 = ImageFont.truetype(font_path, 28)
    f2 = ImageFont.truetype(font_path, 24)
    text_a = int(255 * 0.85)
    d.text((24, 10), f"{name} {surname}", font=f1, fill=(255, 255, 255, text_a))
    d.text((24, 48), index, font=f2, fill=(255, 255, 255, text_a))
    img.save(os.path.join(OUT, "name_overlay.png"))

def banknote(name="cursor_banknote.png", green=(95, 160, 90)):
    # 48x28 u referentnim koordinatama, pravi se 4x pa smanji (antialiasing)
    big = Image.new("RGBA", (192, 112), (0, 0, 0, 0))
    note = Image.new("RGBA", (150, 76), (0, 0, 0, 0))
    d = ImageDraw.Draw(note)
    dark = (16, 36, 14, 255)
    light = (200, 230, 195, 255)
    mid = tuple(max(0, c - 25) for c in green) + (255,)
    d.rounded_rectangle((0, 0, 149, 75), radius=6, fill=green + (255,), outline=dark, width=3)
    d.rounded_rectangle((10, 10, 139, 65), radius=4, outline=light, width=2)
    d.ellipse((50, 18, 100, 58), fill=mid, outline=light, width=2)
    f = ImageFont.truetype(find_font(None), 30)
    d.text((75, 38), "\u20ac", font=f, fill=dark, anchor="mm")
    note = note.rotate(12, expand=True, resample=Image.BICUBIC)
    big.alpha_composite(note, (4, 4))
    big.resize((96, 56), Image.LANCZOS).save(os.path.join(OUT, name))

def icon(name, painter):
    # 28x28 u referentnim koordinatama, crta se na 224x224 pa smanji na 56x56
    S = 224
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    painter(ImageDraw.Draw(img))
    img.resize((56, 56), Image.LANCZOS).save(os.path.join(OUT, name))

DK = (40, 28, 8, 255)

def coin(d):
    d.ellipse((6, 6, 190, 190), fill=(232, 180, 60, 255), outline=DK, width=10)
    d.ellipse((34, 34, 162, 162), outline=(250, 220, 130, 255), width=6)
    f = ImageFont.truetype(find_font(None), 90)
    d.text((98, 100), "\u20ac", font=f, fill=DK, anchor="mm")

def padlock(d):
    d.arc((40, 6, 150, 130), 180, 360, fill=(90, 90, 95, 255), width=20)
    d.line((50, 70, 50, 110), fill=(90, 90, 95, 255), width=20)
    d.line((140, 70, 140, 110), fill=(90, 90, 95, 255), width=20)
    d.rounded_rectangle((14, 100, 176, 210), radius=16, fill=(210, 165, 45, 255), outline=DK, width=10)
    d.ellipse((79, 128, 111, 160), fill=DK)
    d.rectangle((90, 150, 100, 185), fill=DK)

def key(d):
    d.ellipse((6, 6, 100, 100), fill=(232, 180, 60, 255), outline=DK, width=10)
    d.ellipse((38, 38, 68, 68), fill=(0, 0, 0, 0), outline=DK, width=8)
    d.rectangle((90, 42, 214, 64), fill=(232, 180, 60, 255), outline=DK, width=8)
    d.rectangle((166, 62, 186, 100), fill=(232, 180, 60, 255), outline=DK, width=8)
    d.rectangle((194, 62, 212, 86), fill=(232, 180, 60, 255), outline=DK, width=8)

def hand(d):
    skin = (240, 200, 160, 255)
    d.rounded_rectangle((30, 70, 196, 210), radius=34, fill=skin, outline=DK, width=10)
    for x in (72, 113, 154):
        d.line((x, 74, x, 128), fill=DK, width=8)
    d.rounded_rectangle((4, 120, 62, 196), radius=26, fill=skin, outline=DK, width=10)

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", default="Mihajlo")
    ap.add_argument("--surname", default="Milojević")
    ap.add_argument("--index", default="SV xx/2024")
    ap.add_argument("--font", default=None)
    a = ap.parse_args()
    os.makedirs(OUT, exist_ok=True)
    overlay(a.name, a.surname, a.index, find_font(a.font))
    banknote()
    banknote("cursor_banknote_green.png", (70, 200, 110))
    icon("cursor_coin.png", coin)
    icon("cursor_padlock.png", padlock)
    icon("cursor_key.png", key)
    icon("cursor_hand.png", hand)
    print("OK ->", os.path.abspath(OUT))
