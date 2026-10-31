# The icons of the new items

The files of this folder are the icons of the items of the overlay
(`overlay/src/custom_items.cc`). Each icon is a PNG of 30 x 30 pixels, and its
file has the name of the item, with the same letters (`Plasma Core.png` for the
item "Plasma Core"): the game reads the file `sdmc:/sango/items/<name>.bclim`.
The release script (`tools/release.py`) makes the BCLIM files that the game
reads from these PNG files, and it puts them in `sango/items/` of the zip.

To make the two icons of the examples again:

```bash
python tools/make_item_icons.py assets/items
```

You can also draw a PNG with any program. A bigger image shrinks to 30 x 30
pixels, and the tool keeps 15 colors for the opaque pixels.

To test without a release, make the BCLIM file and copy it to the SD card (for
the emulator, the folder `sdmc/sango/items`):

```bash
python tools/png_to_item_icon.py "assets/items/Plasma Core.png" "Plasma Core.bclim"
```
