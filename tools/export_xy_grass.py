#!/usr/bin/env python3
"""Exports the 5 kinds of tall grass of Pokémon X and Y for Alpha Sapphire.

  python tools/export_xy_grass.py <XY a/2/3/0> <out folder> --spica <folder of SPICA>

The file a/2/3/0 of X and Y has the 3D models of the grass (and their motions
of wind). The tool takes the 5 models, converts them to the format of ORAS with
the library SPICA.dll of the program SPICA (Windows only, see
tools/convert_bch.ps1), and writes one pack, grass.pc.

Copy the pack to sdmc:/sango/grass/grass.pc. In the game, the page
Overworld > Tall Grass places the grass on the tiles in front of the player.

These files are data of the game: do not share them.
"""

import argparse
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import garc
from export_xy_trainer_models import convert_models

# The numbers of the 3D models in the pack of X and Y: the grass 0 to 4.
MODEL_PARTS = [7, 11, 15, 19, 23]
# The models that ignore the light in the files of X and Y (the yellow flower
# and the tall grass). The tool gives them the lighting of the model 0.
UNLIT_MODELS = [1, 4]
# The plugin reads the pack in a buffer of this size (kPackCapacity).
PACK_CAPACITY = 0x30000


def main():
  parser = argparse.ArgumentParser(
      description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument("xy_file", help="The file a/2/3/0 of X and Y.")
  parser.add_argument("out_dir")
  parser.add_argument("--spica", help="The folder of SPICA (has SPICA.dll).")
  parser.add_argument("--no-convert", action="store_true",
                      help="Do not convert the format (for tests).")
  args = parser.parse_args()
  if not args.spica and not args.no_convert:
    sys.exit("Give the folder of SPICA with --spica.")

  data = garc.read_garc(args.xy_file)[0][0]
  if data[:1] == b"\x11":
    data = garc.decompress_lz11(data)
  parts = garc.unpack_mm(data) if data[:2] == b"MM" else unpack_tm(data)
  files = {}
  for index, part in enumerate(MODEL_PARTS):
    if parts[part][:4] != b"BCH\x00":
      sys.exit("The part %d is not a 3D model: this is not the file a/2/3/0." %
               part)
    files[index] = parts[part]
  if not args.no_convert:
    convert_models(files, args.spica)
    light_models(files, args.spica)

  pack = garc.pack_mm([files[i] for i in range(len(MODEL_PARTS))], magic=b"PC")
  if len(pack) > PACK_CAPACITY:
    sys.exit("The pack has %d bytes: more than the %d that the plugin reads." %
             (len(pack), PACK_CAPACITY))
  os.makedirs(args.out_dir, exist_ok=True)
  path = os.path.join(args.out_dir, "grass.pc")
  with open(path, "wb") as f:
    f.write(pack)
  print("%s: %d bytes, %d grasses" % (path, len(pack), len(MODEL_PARTS)))


def light_models(files, spica_dir):
  """Makes the models of UNLIT_MODELS lit (see tools/light_grass.ps1)."""
  script = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        "light_grass.ps1")
  with tempfile.TemporaryDirectory() as temp:
    for key, data in files.items():
      with open(os.path.join(temp, "%d.bch" % key), "wb") as f:
        f.write(data)
    result = subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
         script, "-SpicaDir", spica_dir, "-Dir", temp, "-Models",
         ",".join(str(m) for m in UNLIT_MODELS)],
        capture_output=True, text=True)
    if result.returncode != 0:
      sys.exit("The lighting failed: " + result.stdout + result.stderr)
    for key in files:
      with open(os.path.join(temp, "%d.bch" % key), "rb") as f:
        files[key] = f.read()


def unpack_tm(data):
  """Splits a pack of X and Y ("TM"): the same layout as the packs "MM"."""
  import struct
  count = struct.unpack_from("<H", data, 2)[0]
  offsets = struct.unpack_from("<%dI" % (count + 1), data, 4)
  return [data[offsets[i]:offsets[i + 1]] for i in range(count)]


if __name__ == "__main__":
  main()
