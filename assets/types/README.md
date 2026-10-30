# The icons of the new types

The files of this folder are the icons of the types of the overlay
(`overlay/src/custom_types.cc`). Each icon is a PNG of 50 x 20 pixels, and its
file has the name of the type, with a capital (`Cosmic.png` for the type
"Cosmic"): the game reads the file `sdmc:/sango/types/<name>.bclim`. The
release script (`tools/release.py`) makes the BCLIM files that the game reads
from these PNG files, and it puts them in `sango/types/` of the zip.

To make an icon again, or to make a new one:

```bash
python tools/make_type_icon.py COSMIC assets/types/Cosmic.png 2b1f8a 8d3fd0 --stars
python tools/make_type_icon.py SOUND assets/types/Sound.png 0f9d8a 3fe0b8
python tools/make_type_icon.py LIGHT assets/types/Light.png e0a800 fff08a
python tools/make_type_icon.py PLASMA assets/types/Plasma.png c2185b ff7a2f
python tools/make_type_icon.py DREAM assets/types/Dream.png d96bd1 a58bff
python tools/make_type_icon.py TIME assets/types/Time.png 8a5a2b e0b070
```

You can also draw a PNG with any program: keep the size 50 x 20 pixels.

To test without a release, make the BCLIM file and copy it to the SD card:

```bash
python tools/png_to_bclim.py assets/types/Cosmic.png cosmic.bclim
```
