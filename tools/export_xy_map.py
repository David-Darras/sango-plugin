"""Takes a map of Pokemon X/Y for ORAS.

The world of X and Y is made of blocks of 40 x 40 tiles. Each block has a 3D
model of the ground, the walls (collision) and its textures are in the area
packs. ORAS is the same. This tool takes one block or a group of blocks of X or
Y and converts them to the format of ORAS. It writes them in place of blocks of
ORAS, with a new map matrix and the area pack with the textures. The plugin
then loads these files from the SD card instead of the files of the game.

  python tools/export_xy_map.py <xy_a_0_4_1> <xy_a_0_1_3> <oras_a_0_3_9> \
      <oras_a_0_1_4> <oras_a_0_4_0> <out_dir> --region d01 \
      --spica <folder of SPICA>

The archives are files of the RomFS: the map blocks of X or Y ("a/0/4/1"), the
area packs of X or Y ("a/0/1/3"), the map blocks of ORAS ("a/0/3/9"), the area
packs of ORAS ("a/0/1/4") and the map matrices of ORAS ("a/0/4/0"). Choose
what to take with one of these options:

  --block N       one block, by its number in the archive.
  --region NAME   all the blocks of a group. The blocks of X and Y are named
                  with the name of the group and the place in the grid, for
                  example d01_00_00, d01_01_00 (the Santalune Forest), or
                  map01_10_10 (the first region of the world).

The tool writes (in <out_dir>):

  land/<id>.bin     the blocks
  area/<id>.bin     the area pack with the textures
  matrix/<id>.bin   the map matrix: the grid of blocks and the camera limits

Copy the three folders to sdmc:/sango/ (they replace data of ORAS; delete them
to get the game back). The maps 0 to 5 of ORAS are empty: the map 0 uses the
matrix 0, the area 2 and the blocks that the tool writes. In the overlay, use
Overworld > Go To Map with the map 0.

The blocks of ORAS that the tool replaces are the blocks of another map (by
default the blocks of the matrix given by --matrix-base). Use a matrix of
ORAS that has the same size as your group (1 x 1: 100; 2 x 2: 429).

By default the walls and the heights of the ground are the ones of X and Y.
With --collision oras the tool keeps the (flat) collision of ORAS and lowers
the model so that its ground is at the height 0.

These files are data of the game: do not share them.
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

TOOLS = os.path.dirname(os.path.abspath(__file__))
# The parts of a block.
PART_ATTRIBUTES = 0
PART_MODEL = 1
PART_COLLISION = 2
# The part of an area pack with the textures of the area.
AREA_TEXTURE_PART = 11
# The matrix of ORAS to copy, for each size of group.
DEFAULT_MATRIX_BASE = {(1, 1): 100, (2, 2): 429}
BLOCK_NAME = re.compile(rb"([a-z][0-9a-z]{1,10})_(\d\d)_(\d\d)")
TILES = 40
UNITS_PER_TILE = 18


def split_pack(data):
  """Splits a pack of parts. The packs of blocks ("GR") and of areas ("AD")
  have the same layout as the packs "MM"."""
  count = struct.unpack_from("<H", data, 2)[0]
  offsets = struct.unpack_from("<%dI" % (count + 1), data, 4)
  return [data[offsets[i]:offsets[i + 1]] for i in range(count)]


def run_powershell(script, *args):
  result = subprocess.run(
      ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
       os.path.join(TOOLS, script)] + list(args),
      capture_output=True, text=True)
  if result.returncode != 0 or "FAILED" in result.stdout:
    sys.exit("%s failed:\n%s%s" % (script, result.stdout, result.stderr))
  return result.stdout


def find_region(land, name):
  """Returns {(x, z): block number} of the blocks of a group."""
  cells = {}
  for number in range(len(land)):
    parts = split_pack(garc.decompress_lz11(land[number][0]))
    match = BLOCK_NAME.search(parts[PART_MODEL])
    if match and match.group(1).decode() == name:
      cells[(int(match.group(2)), int(match.group(3)))] = number
  if not cells:
    sys.exit("No block is named %s_XX_YY." % name)
  return cells


def find_textures(area_archive, names, folder):
  """Writes the .bch parts of the area packs that hold the textures.

  Returns the paths and the names that were not found. A part is kept when it
  has a name that is not found yet.
  """
  remaining = set(names)
  paths = []
  for index, parts in enumerate(area_archive):
    if not remaining:
      break
    pack = garc.decompress_lz11(parts[0])
    for number, part in enumerate(split_pack(pack)):
      if part[:4] != b"BCH\x00":
        continue
      found = {n for n in remaining if n.encode() + b"\x00" in part}
      if not found:
        continue
      path = os.path.join(folder, "area%d_%d.bch" % (index, number))
      with open(path, "wb") as f:
        f.write(part)
      paths.append(path)
      remaining -= found
  return paths, remaining


def write_file(folder, file_id, data):
  """Writes a file of the folder and adds it to the list of the folder."""
  os.makedirs(folder, exist_ok=True)
  with open(os.path.join(folder, "%d.bin" % file_id), "wb") as f:
    f.write(data)
  index_path = os.path.join(folder, "index.bin")
  entries = {}
  if os.path.isfile(index_path):
    old = open(index_path, "rb").read()
    for i in range(struct.unpack_from("<I", old, 0)[0]):
      key, size = struct.unpack_from("<II", old, 4 + 8 * i)
      entries[key] = size
  entries[file_id] = len(data)
  with open(index_path, "wb") as f:
    f.write(struct.pack("<I", len(entries)))
    for entry in sorted(entries.items()):
      f.write(struct.pack("<II", *entry))


def main():
  parser = argparse.ArgumentParser(description=__doc__,
      formatter_class=argparse.RawDescriptionHelpFormatter)
  for name in ("xy_land", "xy_areas", "oras_land", "oras_areas",
               "oras_matrices", "out_dir"):
    parser.add_argument(name)
  group = parser.add_mutually_exclusive_group(required=True)
  group.add_argument("--block", type=int)
  group.add_argument("--region")
  parser.add_argument("--matrix", type=int, default=0,
                      help="The number of the matrix of ORAS to replace.")
  parser.add_argument("--matrix-base", type=int,
                      help="The matrix of ORAS to copy (it gives the numbers "
                           "of the blocks and the limits of the camera).")
  parser.add_argument("--area", type=int, default=2,
                      help="The number of the area pack of ORAS to replace.")
  parser.add_argument("--area-base", type=int, default=17,
                      help="The area pack of ORAS to copy and to add the "
                           "textures to (17 is Hoenn, route 10).")
  parser.add_argument("--collision", choices=("xy", "oras"), default="xy")
  parser.add_argument("--spica", required=True,
                      help="The folder of SPICA (has SPICA.dll).")
  args = parser.parse_args()

  xy_land = garc.read_garc(args.xy_land)
  xy_areas = garc.read_garc(args.xy_areas)
  oras_land = garc.read_garc(args.oras_land)
  oras_areas = garc.read_garc(args.oras_areas)
  oras_matrices = garc.read_garc(args.oras_matrices)

  if args.block is not None:
    cells = {(0, 0): args.block}
  else:
    cells = find_region(xy_land, args.region)
  min_x = min(x for x, _ in cells)
  min_z = min(z for _, z in cells)
  cells = {(x - min_x, z - min_z): n for (x, z), n in cells.items()}
  width = max(x for x, _ in cells) + 1
  height = max(z for _, z in cells) + 1
  print("Group of %d x %d blocks: %s" % (width, height, cells))

  base_number = args.matrix_base or DEFAULT_MATRIX_BASE.get((width, height))
  if base_number is None:
    sys.exit("Give --matrix-base: a matrix of ORAS of %d x %d blocks."
             % (width, height))
  matrix_parts = split_pack(oras_matrices[base_number][0])
  kind, base_width, base_height = struct.unpack_from("<IHH", matrix_parts[0])
  if kind != 0 or (base_width, base_height) != (width, height):
    sys.exit("The matrix %d is not a grid of %d x %d blocks." %
             (base_number, width, height))
  target_ids = list(struct.unpack_from("<%dH" % (width * height),
                                       matrix_parts[0], 8))

  area_parts = split_pack(garc.decompress_lz11(oras_areas[args.area_base][0]))
  template = split_pack(garc.decompress_lz11(oras_land[0][0]))

  with tempfile.TemporaryDirectory() as temp:
    def path(name, data=None):
      full = os.path.join(temp, name)
      if data is not None:
        with open(full, "wb") as f:
          f.write(data)
      return full

    spica = ["-SpicaDir", args.spica]
    sources = {}
    for cell, number in cells.items():
      parts = split_pack(garc.decompress_lz11(xy_land[number][0]))
      if parts[PART_MODEL][:4] != b"BCH\x00":
        sys.exit("Block %d has no 3D model." % number)
      sources[cell] = (path("block%d.bch" % number, parts[PART_MODEL]), parts)
    models = ",".join(source for source, _ in sources.values())
    names = run_powershell("merge_bch.ps1", *spica, "-Model", models,
                           "-Names").split()
    print("The blocks need %d textures." % len(names), flush=True)
    paths, missing = find_textures(xy_areas, names, temp)
    if missing:
      print("Textures not found:", ", ".join(sorted(missing)))
    textures = ",".join(paths)

    # The same shift for all the blocks, from the first one.
    shift = []
    ground = 0.0
    block_files = {}
    for index, cell in enumerate(sorted(cells, key=lambda c: (c[1], c[0]))):
      source, parts = sources[cell]
      out_path = path("out%d_%d.bch" % cell)
      extra = ["-Strip"]
      if args.collision == "oras":
        extra += ["-ShiftY", shift[0]] if shift else ["-Ground"]
      output = run_powershell("merge_bch.ps1", *spica, "-Model", source,
                              "-Textures", textures, "-Out", out_path, *extra)
      if index == 0:
        for line in output.splitlines():
          if line.startswith("GROUND "):
            ground = float(line.split()[1].replace(",", "."))
          if line.startswith("SHIFT "):
            shift = [line.split()[1].replace(",", ".")]
      block_files[cell] = open(out_path, "rb").read()

    container = path("container.bch", area_parts[AREA_TEXTURE_PART])
    area_out = path("area_out.bch")
    run_powershell("merge_bch.ps1", *spica, "-Model", models,
                   "-Container", container, "-Textures", textures,
                   "-Out", area_out)
    area_model = open(area_out, "rb").read()

  for cell, number in cells.items():
    source, parts = sources[cell]
    new = list(template)
    new[PART_MODEL] = block_files[cell]
    if args.collision == "xy":
      new[PART_ATTRIBUTES] = parts[PART_ATTRIBUTES]
      new[PART_COLLISION] = parts[PART_COLLISION]
    data = garc.compress_lz11(garc.pack_mm(new, magic=b"GR"))
    target = target_ids[cell[0] + cell[1] * width]
    write_file(os.path.join(args.out_dir, "land"), target, data)
    print("Block %d of X/Y -> block %d of ORAS (%d bytes)" %
          (number, target, len(data)))

  area_parts[AREA_TEXTURE_PART] = area_model
  # The game keeps a buffer with the size of the file: a file that is not
  # compressed can be larger than the buffer.
  area = garc.compress_lz11(garc.pack_mm(area_parts, magic=b"AD"))
  write_file(os.path.join(args.out_dir, "area"), args.area, area)

  # The matrix of the map: the grid of the blocks and the limits of the camera.
  # The matrix 0 of ORAS has a camera that cannot move (all its limits are
  # 711), so the tool writes the limits of a normal map.
  limits = struct.pack("<5f", 117.0, height * 720 - 135.0, 63.0,
                       width * 720 - 63.0, 0.0)
  camera = bytearray(matrix_parts[1])
  camera[4:4 + 16] = limits[:16]
  matrix = garc.pack_mm([matrix_parts[0], bytes(camera)], align=4)
  write_file(os.path.join(args.out_dir, "matrix"), args.matrix, matrix)

  x, z = TILES * UNITS_PER_TILE // 2, TILES * UNITS_PER_TILE // 2
  print("Done. Area %d (%d bytes), matrix %d." %
        (args.area, len(area), args.matrix))
  print("Go To Map: Map ID 0, X %d, Height %d, Z %d (the first block)." %
        (x, round(ground) if args.collision == "xy" else 0, z))


if __name__ == "__main__":
  main()
