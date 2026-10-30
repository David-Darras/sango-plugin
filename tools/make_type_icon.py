#!/usr/bin/env python3
"""Draws a type icon in the style of the game: a pill of 50 x 20 pixels.

    make_type_icon.py COSMIC Cosmic.png 3a2a9a 8a46d0 --stars
    make_type_icon.py SOUND Sound.png 12a58f 3fe0b8

The arguments are the text (A to Z), the PNG file, the color at the left and
the color at the right (hexadecimal RGB). Use png_to_bclim.py to make the file
for the plugin (the type icons of the game are 50 x 20 images in RGBA4).
"""

import math
import sys

from PIL import Image

WIDTH = 50
HEIGHT = 20
OUTLINE = (74, 66, 61, 204)

# A font of 5 x 7 pixels. One text for each row, 1 is a pixel.
FONT = {
    "A": "01110 10001 10001 11111 10001 10001 10001",
    "B": "11110 10001 10001 11110 10001 10001 11110",
    "C": "01110 10001 10000 10000 10000 10001 01110",
    "D": "11100 10010 10001 10001 10001 10010 11100",
    "E": "11111 10000 10000 11110 10000 10000 11111",
    "F": "11111 10000 10000 11110 10000 10000 10000",
    "G": "01110 10001 10000 10111 10001 10001 01111",
    "H": "10001 10001 10001 11111 10001 10001 10001",
    "I": "01110 00100 00100 00100 00100 00100 01110",
    "J": "00111 00010 00010 00010 00010 10010 01100",
    "K": "10001 10010 10100 11000 10100 10010 10001",
    "L": "10000 10000 10000 10000 10000 10000 11111",
    "M": "10001 11011 10101 10101 10001 10001 10001",
    "N": "10001 11001 10101 10011 10001 10001 10001",
    "O": "01110 10001 10001 10001 10001 10001 01110",
    "P": "11110 10001 10001 11110 10000 10000 10000",
    "Q": "01110 10001 10001 10001 10101 10010 01101",
    "R": "11110 10001 10001 11110 10100 10010 10001",
    "S": "01111 10000 10000 01110 00001 00001 11110",
    "T": "11111 00100 00100 00100 00100 00100 00100",
    "U": "10001 10001 10001 10001 10001 10001 01110",
    "V": "10001 10001 10001 10001 10001 01010 00100",
    "W": "10001 10001 10001 10101 10101 11011 10001",
    "X": "10001 10001 01010 00100 01010 10001 10001",
    "Y": "10001 10001 01010 00100 00100 00100 00100",
    "Z": "11111 00001 00010 00100 01000 10000 11111",
}


def parse_color(text):
    return tuple(int(text[i:i + 2], 16) for i in (0, 2, 4))


def mix(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


def lighten(color, amount):
    return mix(color, (255, 255, 255), amount)


def darken(color, amount):
    return mix(color, (0, 0, 0), amount)


def pill_distance(x, y):
    """The distance from a pixel to the edge of the pill (< 0: inside)."""
    radius = (HEIGHT - 2) / 2.0
    left = radius
    right = WIDTH - radius
    cx = min(max(x + 0.5, left), right)
    cy = 1 + radius
    return math.hypot(x + 0.5 - cx, y + 0.5 - cy) - radius


def draw_text(image, text, color_left):
    """Draws the text with letters of 5 x 7 pixels and stems of 2 pixels, as the game does: a
    white letter with a dark shadow to the right and to the bottom."""
    glyphs = [[row for row in FONT[c].split()] for c in text]
    # The space between the letters grows when the text is short.
    advance = min(8, 40 // len(glyphs))
    width = (len(glyphs) - 1) * advance + 5
    start_x = (WIDTH - width) // 2
    start_y = 6
    mask = {}
    for index, glyph in enumerate(glyphs):
        for gy, row in enumerate(glyph):
            for gx, bit in enumerate(row):
                if bit == "1":
                    mask[(start_x + index * advance + gx, start_y + gy)] = gy
    # A little bolder: the left stem of a letter (a pixel in the first column of
    # the letter, with a pixel above and below it) gets a second pixel to its
    # left... so the letter keeps its shape.
    for index in range(len(glyphs)):
        left = start_x + index * advance
        for (x, y), gy in list(mask.items()):
            if x == left and (x, y - 1) in mask and (x, y + 1) in mask:
                mask[(x - 1, y)] = gy
    dark = (74, 66, 61, 255)
    for (x, y) in list(mask):
        for dx, dy in ((1, 1), (0, 1), (1, 0)):
            if (x + dx, y + dy) not in mask:
                image.putpixel((x + dx, y + dy), dark)
    for (x, y), gy in mask.items():
        shade = 255 - gy * 9
        image.putpixel((x, y), (shade, shade, min(255, shade + 4), 255))


def make_icon(text, color_left, color_right, stars):
    image = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    for y in range(HEIGHT):
        for x in range(WIDTH):
            d = pill_distance(x, y)
            if d > 0:
                continue
            t = x / (WIDTH - 1)
            base = mix(color_left, color_right, t)
            if d > -1:
                image.putpixel((x, y), OUTLINE)
            elif d > -2:
                image.putpixel((x, y), lighten(base, 0.8) + (255,))
            elif d > -3:
                image.putpixel((x, y), lighten(base, 0.4) + (255,))
            else:
                # The top is lighter and the bottom is darker.
                shade = (y - 3) / (HEIGHT - 7)
                tone = lighten(base, 0.12) if shade < 0.35 else (
                    darken(base, 0.1) if shade > 0.75 else base)
                image.putpixel((x, y), tone + (255,))
    if stars:
        # Small stars in the corners, away from the letters.
        for (sx, sy, big) in ((4, 5, True), (45, 14, True), (46, 5, False),
                              (3, 14, False)):
            star = (255, 255, 255, 255)
            image.putpixel((sx, sy), star)
            if big:
                for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                    image.putpixel((sx + dx, sy + dy), (255, 240, 170, 255))
    draw_text(image, text, color_left)
    return image


def main():
    if len(sys.argv) < 5:
        raise SystemExit(__doc__)
    text = sys.argv[1].upper()
    for c in text:
        if c not in FONT:
            raise SystemExit("the text can have the letters A to Z only")
    image = make_icon(text, parse_color(sys.argv[3]), parse_color(sys.argv[4]),
                      "--stars" in sys.argv[5:])
    image.save(sys.argv[2])


if __name__ == "__main__":
    main()
