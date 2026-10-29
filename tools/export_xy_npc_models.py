"""Takes the overworld models of the characters of Pokemon X/Y for ORAS.

Alpha Sapphire has the slots of the characters of X and Y (Serena, Calem,
Professor Sycamore...), but their models are small placeholders (an empty file
or a Poké Ball). This tool reads the
models in the archive of X or Y and writes them for the SD card. The plugin
then fills the empty slots of the game with them.

  python tools/export_xy_npc_models.py <xy_a_0_2_1> <oras_a_0_2_1> <out_dir> \
      --spica <folder of SPICA>

The two first arguments are the archive of the overworld models of the two
games: the file "a/0/2/1" of the RomFS. The tool writes:

  <out_dir>/overworld_model/index.bin   the list of the models
  <out_dir>/overworld_model/<id>.bin    one model (the id of overworld::ModelId)

Copy the folder "overworld_model" to sdmc:/sango/. Then use the id with
overworld::ModelId, for example in the page Model Loader of the overlay.

The models of X and Y have an older 3D format than the models of ORAS. The
tool converts them with the library SPICA.dll of the program SPICA (Windows
only, see tools/convert_bch.ps1). The option --spica gives its folder.

The tool fills only the slots that are empty in ORAS. The models of the game
stay as they are. These files are data of the game: do not share them.
"""

import argparse
import os
import re
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import garc

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# A model smaller than this (compressed) is a placeholder.
EMPTY_LIMIT = 20000


def read_names():
  """Returns the names of overworld::ModelId, by value."""
  names = {}
  path = os.path.join(REPO, "lib", "include", "overworld", "constant",
                      "model.h")
  pattern = re.compile(r"^\s*k(\w+)\s*=\s*(0x[0-9a-fA-F]+|\d+)\s*,")
  for line in open(path, encoding="utf-8"):
    match = pattern.match(line)
    if match:
      names.setdefault(int(match.group(2), 0), match.group(1))
  return names


def convert_models(packs, spica_dir):
  """Converts the 3D models of the packs to the format of ORAS.

  `packs` is {id: list of parts}. A part that starts with "BCH" is a 3D
  model. The function changes the parts in place.
  """
  script = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        "convert_bch.ps1")
  with tempfile.TemporaryDirectory() as temp:
    source = os.path.join(temp, "in")
    target = os.path.join(temp, "out")
    os.makedirs(source)
    for model_id, parts in packs.items():
      for index, part in enumerate(parts):
        if part[:4] == b"BCH\x00":
          with open(os.path.join(source, "%d_%d.bch" % (model_id, index)),
                    "wb") as f:
            f.write(part)
    result = subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
         script, "-SpicaDir", spica_dir, "-InDir", source, "-OutDir", target],
        capture_output=True, text=True)
    if result.returncode != 0 or "FAILED" in result.stdout:
      sys.exit("The conversion failed:\n" + result.stdout + result.stderr)
    for model_id, parts in packs.items():
      for index, part in enumerate(parts):
        path = os.path.join(target, "%d_%d.bch" % (model_id, index))
        if os.path.isfile(path):
          parts[index] = open(path, "rb").read()


def main():
  parser = argparse.ArgumentParser(description=__doc__,
      formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument("xy_archive")
  parser.add_argument("oras_archive")
  parser.add_argument("out_dir")
  parser.add_argument("--spica", help="The folder of SPICA (has SPICA.dll).")
  parser.add_argument("--no-convert", action="store_true",
                      help="Do not convert the format (for tests).")
  parser.add_argument("--ids", help="Only these ids, for example 1,2,0x10.")
  parser.add_argument("--list", action="store_true",
                      help="Show the models that can be added, then stop.")
  args = parser.parse_args()

  xy = garc.read_garc(args.xy_archive)
  oras = garc.read_garc(args.oras_archive)
  names = read_names()
  wanted = None
  if args.ids:
    wanted = {int(x, 0) for x in args.ids.split(",")}

  models = []
  for model_id in range(min(len(xy), len(oras))):
    if wanted is not None and model_id not in wanted:
      continue
    if not xy[model_id] or len(xy[model_id][0]) < EMPTY_LIMIT:
      continue
    # The slots of ORAS without a real model have a small placeholder (an
    # empty file or a Poké Ball).
    if len(oras[model_id][0]) >= EMPTY_LIMIT:
      continue
    models.append(model_id)

  if args.list:
    for model_id in models:
      print("0x%02x  %s" % (model_id, names.get(model_id, "?")))
    print(len(models), "models")
    return
  if not args.spica and not args.no_convert:
    sys.exit("Give the folder of SPICA with --spica.")

  packs = {m: garc.unpack_mm(garc.decompress_lz11(xy[m][0])) for m in models}
  # ORAS has more motions than X and Y. The game asks for them by their number.
  motion_count = max(
      struct.unpack_from("<I", garc.unpack_mm(
          garc.decompress_lz11(oras[m][0]))[1], 0)[0]
      for m in range(0xab, 0xb3))
  for parts in packs.values():
    if len(parts) > 1:
      parts[1] = garc.extend_motion_pack(parts[1], motion_count)
  if not args.no_convert:
    print("Converting the 3D format of %d models..." % len(models), flush=True)
    convert_models(packs, args.spica)

  folder = os.path.join(args.out_dir, "overworld_model")
  os.makedirs(folder, exist_ok=True)
  index = []
  for model_id in models:
    # The file is a compressed file, as the files of the archive.
    data = garc.compress_lz11_store(garc.pack_mm(packs[model_id]))
    with open(os.path.join(folder, "%d.bin" % model_id), "wb") as f:
      f.write(data)
    index.append((model_id, len(data)))
    print("0x%02x %-32s %8d bytes" % (model_id, names.get(model_id, "?"),
                                       len(data)))
  with open(os.path.join(folder, "index.bin"), "wb") as f:
    f.write(struct.pack("<I", len(index)))
    for entry in sorted(index):
      f.write(struct.pack("<II", *entry))
  print("%d models written in %s" % (len(index), folder))


if __name__ == "__main__":
  main()
