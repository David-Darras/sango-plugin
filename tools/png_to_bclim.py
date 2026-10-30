#!/usr/bin/env python3
"""Converts a PNG to a BCLIM image (RGBA4) that the layouts of the game read.

The type icons of the game are 50 x 20 pixels, in a texture of 64 x 32. This
tool makes the same kind of file, so the plugin can give the game a new type
icon (see lib/src/renderer/type_icon.cc).

    png_to_bclim.py Cosmic.png Cosmic.bclim

The image goes in the corner of a texture whose sides are the next powers of 2
(8 pixels at least): the game wants this size for the memory of the GPU, and
the footer keeps the real size of the image (50 x 20 for a type icon). The rows
start at the top of the image, and the padding is under it.
"""

import struct
import sys

from PIL import Image

FORMAT_RGBA4 = 8
FOOTER_SIZE = 0x28


def texture_side(value):
    side = 8
    while side < value:
        side *= 2
    return side


def tile_offset(x, y):
    """The place of a pixel inside a tile of 8 x 8 (the bits of x and y alternate)."""
    return ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
            ((x & 4) << 2) | ((y & 4) << 3))


def convert(image):
    image = image.convert("RGBA")
    width, height = image.size
    if width > 512 or height > 512:
        raise SystemExit("the image is too big (512 x 512 at most)")
    side_x = texture_side(width)
    side_y = texture_side(height)

    data = bytearray(side_x * side_y * 2)
    tiles_per_row = side_x // 8
    pixels = image.load()
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                r = g = b = 0
            raw = ((round(r / 17) << 12) | (round(g / 17) << 8) |
                   (round(b / 17) << 4) | round(a / 17))
            # The first row of the texture is the top row of the image: the
            # padding is under the image and at its right.
            memory_y = y
            tile = (memory_y // 8) * tiles_per_row + x // 8
            index = tile * 64 + tile_offset(x % 8, memory_y % 8)
            struct.pack_into("<H", data, index * 2, raw)

    footer = bytearray()
    footer += b"CLIM"
    footer += struct.pack("<HHI", 0xFEFF, 0x14, 0x02020000)
    footer += struct.pack("<I", len(data) + FOOTER_SIZE)  # the size of the file
    footer += struct.pack("<HH", 1, 0)  # one block
    footer += b"imag"
    footer += struct.pack("<I", 0x10)
    footer += struct.pack("<HH", width, height)
    footer += struct.pack("<I", FORMAT_RGBA4)
    footer += struct.pack("<I", len(data))
    assert len(footer) == FOOTER_SIZE
    return bytes(data) + bytes(footer)


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    with open(sys.argv[2], "wb") as out:
        out.write(convert(Image.open(sys.argv[1])))


if __name__ == "__main__":
    main()
