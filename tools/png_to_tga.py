"""Make a TGA image for the plugin from a PNG image (or a JPG, a BMP...).

The plugin reads TGA images without compression, with 32 bits for each pixel
(see ui::Image). The images of the menu go to sdmc:/sango/:
menu_top.tga (400 x 240) and menu_bottom.tga (320 x 240).

Usage:
    python tools/png_to_tga.py menu_top.png             -> menu_top.tga
    python tools/png_to_tga.py menu_top.png out.tga     -> out.tga
    python tools/png_to_tga.py photo.jpg --size 400x240 -> canvas 400x240, image at the top-left

The script needs Pillow: pip install pillow
"""

import argparse
import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("This script needs Pillow: pip install pillow")

MAX_SIZE = 512


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("input", help="the PNG image (or another format)")
    parser.add_argument("output", nargs="?", help="the TGA image")
    parser.add_argument("--size", help="canvas size, for example 400x240")
    args = parser.parse_args()

    image = Image.open(args.input).convert("RGBA")

    if args.size:
        width, height = (int(v) for v in args.size.lower().split("x"))
        
        canvas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        
        pos_x = 0
        pos_y = 0
        
        canvas.paste(image, (pos_x, pos_y), image)
        
        image = canvas

    if image.width > MAX_SIZE or image.height > MAX_SIZE:
        sys.exit("The image is %dx%d: the plugin accepts %dx%d at most. "
                 "Use --size." % (image.width, image.height, MAX_SIZE,
                                  MAX_SIZE))

    output = args.output or os.path.splitext(args.input)[0] + ".tga"
    # 32 bits for each pixel, without compression (RLE off).
    image.save(output, format="TGA", rle=False)
    print("%s: %dx%d, 32 bits" % (output, image.width, image.height))


if __name__ == "__main__":
    main()