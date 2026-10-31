#!/usr/bin/env python3
"""Converts a PNG to an item icon (a BCLIM image with a palette of 16 colors).

The item icons of the game are 30 x 30 pixels, in a texture of 32 x 32. The
plugin reads the file `sdmc:/sango/items/<name>.bclim` for the new item with
the same name (see battle::ItemExtension).

    png_to_item_icon.py plasma_core.png "Plasma Core.bclim"

The image goes in the corner of the texture. The colors have the format
RGBA5551, and a pixel that is almost transparent uses the color 0 (white, like
the icons of the game). The tool
keeps 15 colors for the opaque pixels.
"""

import struct
import sys

from PIL import Image

SIDE = 30
TEXTURE = 32
FORMAT_RGBA5551 = 7
FOOTER_SIZE = 0x28
DATA_SIZE = TEXTURE * TEXTURE * 2


def tile_offset(x, y):
    """The place of a pixel inside a tile of 8 x 8 (the bits of x and y alternate)."""
    return ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
            ((x & 4) << 2) | ((y & 4) << 3))


def color5551(r, g, b, a):
    return ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | (1 if a else 0)


def convert(image):
    image = image.convert("RGBA")
    if image.size != (SIDE, SIDE):
        image = image.resize((SIDE, SIDE), Image.LANCZOS)

    alpha = image.getchannel("A")
    opaque = [px >= 128 for px in alpha.tobytes()]

    # 15 colors for the opaque pixels. Color 0 is transparent.
    rgb = image.convert("RGB")
    quantized = rgb.quantize(colors=15, method=Image.MEDIANCUT, dither=Image.NONE)
    palette = quantized.getpalette()[:15 * 3]
    indexes = list(quantized.tobytes())

    # Color 0 is transparent. Like the icons of the game, it is white (not black):
    # the game draws the icon with a smooth filter, and a black transparent color
    # makes dark dots around the outline.
    colors = [0xFFFE]
    for i in range(15):
        r, g, b = palette[i * 3:i * 3 + 3] if i * 3 + 2 < len(palette) else (0, 0, 0)
        colors.append(color5551(r, g, b, True))

    texture = [0] * (TEXTURE * TEXTURE)
    for y in range(SIDE):
        for x in range(SIDE):
            i = y * SIDE + x
            value = indexes[i] + 1 if opaque[i] else 0
            tile = (y // 8) * (TEXTURE // 8) + x // 8
            texture[tile * 64 + tile_offset(x % 8, y % 8)] = value

    data = bytearray()
    data += struct.pack("<HH", 2, 16)
    for c in colors:
        data += struct.pack("<H", c)
    for i in range(0, len(texture), 2):
        data.append((texture[i] << 4) | texture[i + 1])
    data += bytes(DATA_SIZE - len(data))

    footer = bytearray()
    footer += b"CLIM"
    footer += struct.pack("<HHI", 0xFEFF, 0x14, 0x02020000)
    footer += struct.pack("<I", len(data) + FOOTER_SIZE)
    footer += struct.pack("<HH", 1, 0)
    footer += b"imag"
    footer += struct.pack("<I", 0x10)
    footer += struct.pack("<HH", SIDE, SIDE)
    footer += struct.pack("<I", FORMAT_RGBA5551)
    footer += struct.pack("<I", len(data))
    assert len(footer) == FOOTER_SIZE
    return bytes(data) + bytes(footer)


def decode(data):
    """Reads an item icon back to an RGBA image (to check the tool)."""
    colors_count = struct.unpack_from("<H", data, 2)[0]
    colors = struct.unpack_from("<%dH" % colors_count, data, 4)
    indexes = data[4 + colors_count * 2:]
    image = Image.new("RGBA", (SIDE, SIDE))
    for y in range(SIDE):
        for x in range(SIDE):
            tile = (y // 8) * (TEXTURE // 8) + x // 8
            i = tile * 64 + tile_offset(x % 8, y % 8)
            index = (indexes[i // 2] >> (0 if i & 1 else 4)) & 0xF
            c = colors[index]
            image.putpixel((x, y), (((c >> 11) & 31) << 3, ((c >> 6) & 31) << 3,
                                    ((c >> 1) & 31) << 3, 255 if c & 1 else 0))
    return image


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    with open(sys.argv[2], "wb") as out:
        out.write(convert(Image.open(sys.argv[1])))


if __name__ == "__main__":
    main()
