"""Converts the music of Pokemon X/Y (.aac) to DSP-ADPCM .bcstm files for ORAS.

The game ORAS reads its music as DSP-ADPCM ".bcstm" files. The game X/Y reads
".aac" files. This tool converts a whole folder of X/Y music and writes the
list for the overlay:

  python aac_to_bcstm.py <xy_sound_folder> <out_folder>

It writes <out_folder>/xy_<name>.dspadpcm.bcstm for each .aac file and
<out_folder>/list.txt (one line per file: the file name, "|", the Bulbapedia
title of the music). Copy the content of the out folder to sdmc:/sango/bgm/.
The page Sound of the overlay then has the entry "SD Music".

To add your own music, use add_music.py.
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bcstm_tools
import music_names


def main():
  parser = argparse.ArgumentParser(description=__doc__,
      formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument("input", help="The folder of the .aac files.")
  parser.add_argument("out_dir")
  parser.add_argument("--only", help="Convert only the names that contain "
                      "this text.")
  parser.add_argument("--ffmpeg")
  parser.add_argument("--vgaudio")
  args = parser.parse_args()

  os.makedirs(args.out_dir, exist_ok=True)
  names = []
  for file in sorted(os.listdir(args.input)):
    stem, ext = os.path.splitext(file)
    if ext.lower() != ".aac":
      continue
    if args.only and args.only not in stem:
      continue
    name = "xy_" + stem
    if len(name) >= 40:
      print("Skipped (name too long):", file)
      continue
    out_path = os.path.join(args.out_dir, name + bcstm_tools.EXTENSION)
    if not os.path.isfile(out_path):
      print("Converting", file, flush=True)
      bcstm_tools.convert(os.path.join(args.input, file), out_path, "full",
                          args.ffmpeg, args.vgaudio)
    names.append((music_names.xy_title(stem), name))
  # One line for each file: the name of the file, "|", the title.
  names.sort()
  with open(os.path.join(args.out_dir, "list.txt"), "w", newline="\n") as f:
    f.write("".join("%s|%s\n" % (name, title) for title, name in names))
  print("%d files, list.txt written." % len(names))


if __name__ == "__main__":
  main()
