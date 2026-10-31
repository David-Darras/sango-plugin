#!/usr/bin/env python3
"""Draws the icons of the example items (assets/items): Plasma Core, Dream
Pillow, Shiny Switch, Ability Switch, Evolution Prism, Knife,
Devolution Stone and Philosopher Stone. Each icon is a PNG of 30 x 30 pixels,
in the style of the item icons of the game (a dark outline, three tones, a
light spot).

    make_item_icons.py assets/items
"""

import math
import os
import sys

from PIL import Image

SIDE = 30
OUTLINE = (40, 30, 50, 255)


def put(image, x, y, color):
    if 0 <= x < SIDE and 0 <= y < SIDE:
        image.putpixel((x, y), color)


def disc(image, cx, cy, radius, color):
    for y in range(SIDE):
        for x in range(SIDE):
            if (x - cx) ** 2 + (y - cy) ** 2 <= radius ** 2:
                put(image, x, y, color)


def plasma_core():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    cx, cy = 14.5, 15.0
    disc(image, cx, cy, 12.5, OUTLINE)
    disc(image, cx, cy, 11.0, (120, 30, 150, 255))
    disc(image, cx - 0.5, cy - 0.5, 8.5, (214, 60, 190, 255))
    disc(image, cx - 1.5, cy - 1.5, 5.5, (255, 120, 200, 255))
    disc(image, cx - 2.5, cy - 2.5, 3.0, (255, 215, 235, 255))
    # Arcs of plasma around the core.
    for angle in range(0, 360, 45):
        a = math.radians(angle)
        x = int(round(cx + 13.0 * math.cos(a)))
        y = int(round(cy + 13.0 * math.sin(a)))
        put(image, x, y, (255, 150, 60, 255))
    for x, y in ((8, 7), (9, 6), (20, 21), (21, 22)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def dream_pillow():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A soft rounded pillow.
    for y in range(SIDE):
        for x in range(SIDE):
            nx = (x - 14.5) / 13.5
            ny = (y - 15.0) / 9.5
            d = abs(nx) ** 3.2 + abs(ny) ** 3.2
            if d <= 1.0:
                put(image, x, y, OUTLINE)
            if d <= 0.78:
                shade = (166, 120, 235, 255)
                if ny > 0.35:
                    shade = (120, 80, 200, 255)
                elif ny < -0.35:
                    shade = (205, 170, 250, 255)
                put(image, x, y, shade)
    # Stitches at the four corners.
    for x, y in ((4, 8), (5, 9), (25, 8), (24, 9), (4, 22), (5, 21), (25, 22), (24, 21)):
        put(image, x, y, (90, 55, 160, 255))
    # A star and a small moon in the center.
    for x, y in ((14, 11), (13, 12), (14, 12), (15, 12), (14, 13), (14, 14), (15, 15)):
        put(image, x, y, (255, 240, 140, 255))
    for x, y in ((9, 14), (9, 15), (10, 16), (11, 16)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def shiny_switch():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A faceted gem: a diamond with the colors of the rainbow in bands.
    colors = [(255, 90, 90), (255, 170, 70), (255, 235, 90), (110, 220, 110),
              (90, 180, 255), (150, 110, 255)]
    for y in range(SIDE):
        for x in range(SIDE):
            d = abs(x - 14.5) / 13.0 + abs(y - 15.0) / 13.0
            if d <= 1.0:
                put(image, x, y, OUTLINE)
            if d <= 0.82:
                band = int((x + y) / 60.0 * len(colors) * 1.9) % len(colors)
                r, g, b = colors[band]
                shade = 0.8 + 0.2 * (1.0 - (y / SIDE))
                put(image, x, y, (int(r * shade), int(g * shade), int(b * shade), 255))
    for x, y in ((11, 9), (12, 9), (11, 10), (12, 11), (17, 21)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def ability_switch():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A capsule on a slant: two halves, one dark and one light.
    for y in range(SIDE):
        for x in range(SIDE):
            u = (x - 14.5) * 0.7071 + (y - 15.0) * -0.7071
            v = (x - 14.5) * 0.7071 + (y - 15.0) * 0.7071
            end1 = (v - 7.0) ** 2 + u ** 2 <= 5.5 ** 2 and v > 7.0
            end2 = (v + 7.0) ** 2 + u ** 2 <= 5.5 ** 2 and v < -7.0
            inside = (abs(v) <= 7.0 and abs(u) <= 5.5) or end1 or end2
            outer = (abs(v) <= 7.0 and abs(u) <= 7.0) or \
                    (v - 7.0) ** 2 + u ** 2 <= 7.0 ** 2 and v > 7.0 or \
                    (v + 7.0) ** 2 + u ** 2 <= 7.0 ** 2 and v < -7.0
            if outer:
                put(image, x, y, OUTLINE)
            if inside:
                if v > 0:
                    put(image, x, y, (240, 90, 120, 255) if u > -1.0 else (255, 150, 170, 255))
                else:
                    put(image, x, y, (245, 245, 250, 255) if u > -1.0 else (255, 255, 255, 255))
    for x, y in ((10, 10), (11, 9), (9, 11)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def evolution_prism():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A hexagonal crystal with a spiral: the colors of the evolution stones.
    for y in range(SIDE):
        for x in range(SIDE):
            nx = (x - 14.5) / 12.5
            ny = (y - 15.0) / 13.5
            d = max(abs(nx), abs(ny) * 0.87 + abs(nx) * 0.5)
            if d <= 1.0:
                put(image, x, y, OUTLINE)
            if d <= 0.8:
                if ny < -0.45:
                    color = (170, 230, 255, 255)
                elif nx < -0.2:
                    color = (90, 170, 245, 255)
                elif nx > 0.35:
                    color = (60, 120, 210, 255)
                else:
                    color = (120, 195, 250, 255)
                put(image, x, y, color)
    # A spiral of small points inside the crystal.
    for i in range(0, 40):
        a = i * 0.45
        r = 1.2 + i * 0.22
        x = int(round(14.5 + r * math.cos(a)))
        y = int(round(15.5 + r * math.sin(a)))
        put(image, x, y, (255, 245, 170, 255))
    for x, y in ((9, 8), (10, 8), (9, 9)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def knife():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A knife on a slant: a steel blade, a dark guard and a purple handle.
    for y in range(SIDE):
        for x in range(SIDE):
            u = (x - 14.5) * 0.7071 + (y - 15.0) * 0.7071   # along the knife
            v = (x - 14.5) * 0.7071 - (y - 15.0) * 0.7071   # across the knife
            # The blade goes from u = -14 (the tip) to u = 2; it is narrow, and
            # it gets thin near the tip.
            width = 2.6 * min(1.0, (u + 14.0) / 14.0) if u >= -14.0 else -1.0
            blade = -14.0 <= u <= 2.0 and abs(v) <= width
            edge = -15.0 <= u <= 2.0 and abs(v) <= width + 1.5
            guard = 2.0 < u <= 4.5 and abs(v) <= 6.0
            handle = 4.5 < u <= 12.0 and abs(v) <= 2.6
            pommel = 12.0 < u <= 14.0 and abs(v) <= 3.4
            if edge or guard or handle or pommel:
                put(image, x, y, OUTLINE)
            if blade:
                put(image, x, y, (225, 232, 245, 255) if v < 0.5 else (150, 160, 190, 255))
            if 2.0 < u <= 4.5 and abs(v) <= 4.7:
                put(image, x, y, (90, 60, 130, 255))
            if 4.5 < u <= 12.0 and abs(v) <= 1.3:
                put(image, x, y, (130, 80, 190, 255))
            if 12.0 < u <= 14.0 and abs(v) <= 2.0:
                put(image, x, y, (190, 90, 230, 255))
    # Drops of blood at the tip of the blade.
    for x, y in ((3, 6), (3, 7), (5, 9)):
        put(image, x, y, (200, 30, 50, 255))
    for x, y in ((16, 7), (17, 6)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def philosopher_stone():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A red faceted stone with a golden ring: the stone of the legend.
    for y in range(SIDE):
        for x in range(SIDE):
            nx = (x - 14.5) / 12.5
            ny = (y - 15.0) / 12.5
            d = abs(nx) + abs(ny)
            if d <= 1.15 and max(abs(nx), abs(ny)) <= 0.9:
                put(image, x, y, OUTLINE)
            if d <= 0.98 and max(abs(nx), abs(ny)) <= 0.78:
                if ny < -0.35:
                    color = (255, 130, 120, 255)
                elif nx < 0:
                    color = (225, 40, 60, 255)
                else:
                    color = (160, 20, 50, 255)
                put(image, x, y, color)
    for angle in range(0, 360, 12):
        a = math.radians(angle)
        put(image, int(round(14.5 + 13.5 * math.cos(a))),
            int(round(15.0 + 13.5 * math.sin(a))), (255, 215, 80, 255))
    for x, y in ((10, 9), (11, 9), (10, 10), (18, 20)):
        put(image, x, y, (255, 255, 255, 255))
    return image


def devolution_stone():
    image = Image.new("RGBA", (SIDE, SIDE), (0, 0, 0, 0))
    # A grey-blue round stone with a white arrow that turns back.
    cx, cy = 14.5, 15.0
    disc(image, cx, cy, 13.0, OUTLINE)
    disc(image, cx, cy, 11.5, (70, 90, 130, 255))
    disc(image, cx - 0.5, cy - 0.5, 9.5, (110, 135, 185, 255))
    disc(image, cx - 1.5, cy - 1.5, 6.5, (160, 185, 225, 255))
    # An arrow that turns anticlockwise, with its head at the left.
    for angle in range(40, 300, 6):
        a = math.radians(angle)
        for r in (5.0, 6.0):
            put(image, int(round(cx + r * math.cos(a))),
                int(round(cy + r * math.sin(a))), (255, 255, 255, 255))
    for x, y in ((8, 14), (9, 13), (10, 12), (7, 15), (6, 16), (9, 16), (10, 17),
                 (8, 17), (7, 14), (9, 14), (8, 15)):
        put(image, x, y, (255, 255, 255, 255))
    for x, y in ((9, 8), (10, 8), (9, 9)):
        put(image, x, y, (230, 245, 255, 255))
    return image


def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    folder = sys.argv[1]
    os.makedirs(folder, exist_ok=True)
    plasma_core().save(os.path.join(folder, "Plasma Core.png"))
    dream_pillow().save(os.path.join(folder, "Dream Pillow.png"))
    shiny_switch().save(os.path.join(folder, "Shiny Switch.png"))
    ability_switch().save(os.path.join(folder, "Ability Switch.png"))
    evolution_prism().save(os.path.join(folder, "Evolution Prism.png"))
    knife().save(os.path.join(folder, "Knife.png"))
    devolution_stone().save(os.path.join(folder, "Devolution Stone.png"))
    philosopher_stone().save(os.path.join(folder, "Philosopher Stone.png"))


if __name__ == "__main__":
    main()
