#!/usr/bin/env python3
"""Exports the battle backgrounds of Pokémon X and Y for Alpha Sapphire.

  python tools/export_xy_battle_backgrounds.py <XY a/1/4/6> <out folder> \
      --spica <folder of SPICA> --oras-archive <ORAS a/1/3/6>

The archive a/1/4/6 of X and Y has the pieces of the battle backgrounds. A
background has several pieces, and each kind of piece has its own models:

  G  the ground (a file with the signature "BG" and 4 3D models)
  O  the plate under the Pokémon (a file "BG")
  V  a piece "BG" that the versus battles use
  F  the far ground (one 3D model)
  B  the background behind the far ground (one 3D model)

The tool converts the models to the format of ORAS with the library SPICA.dll
of the program SPICA (Windows only, see tools/convert_bch.ps1), and writes each
piece as a new file after the 158 files of the archive of ORAS (a/1/3/6): the
piece number n (0 is the first) is the file 158 + n.

Copy the folder "battle_background" to sdmc:/sango/ and the folder "names" to
sdmc:/sango/. The file list.txt shows the kind and the names of each piece, and
oras_list.txt shows the same for the files of ORAS.

These files are data of the game: do not share them.
"""

import argparse
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import garc
from export_xy_trainer_models import convert_models

# The number of files in the archive of ORAS: the first new file.
FIRST_FILE = 158
# The English name of each name that the files of the backgrounds use. The
# names come from the files of the game: kusa is grass, yama is mountain...
STEMS = {
    "kusa": "Grass", "sea": "Sea", "namk": "Avenue", "4hi": "Malva (Fire)",
    "4miz": "Siebold (Water)", "4hag": "Wikstrom (Steel)",
    "4dor": "Drasna (Dragon)", "roji": "Alley", "t02": "Aquacorde Town",
    "heya": "Room", "yama": "Mountain", "vs1": "Link Battle",
    "do_0": "Cave", "do_k": "Mossy Cave", "do_m": "Reflection Cave",
    "do_y": "Snow Cave", "city": "City", "town": "Town",
    "gym1": "Santalune Gym", "gym2": "Cyllage Gym", "gym3": "Shalour Gym",
    "gym4": "Coumarine Gym", "gym5": "Lumiose Gym", "gym6": "Laverre Gym",
    "gym7": "Anistar Gym", "gym8": "Snowbelle Gym", "mizu": "Water",
    "pala": "Palace", "yuki": "Snow", "hama": "Shore", "fact": "Factory",
    "mori": "Forest", "akai": "Plain Far Ground", "cham": "Champion",
    "s_mo": "Small Forest", "boss": "Flare Boss", "p_x": "Xerneas",
    "p_y": "Yveltal", "aki": "Autumn Grass", "siba": "Plain",
    "kura": "Dark Room", "himi": "Flare Base", "vs2": "Championship 1",
    "vs3": "Championship 2", "end1": "Parade", "sky1": "Sky", "mega": "Mega",
    "ak_m": "Autumn Water", "ak_h": "Autumn Plain", "hasi": "Bridge",
    "numa": "Marsh",
    "iwa1": "Rocks 1", "iwa2": "Rocks 2", "iwa3": "Rocks 3",
    "iwa4": "Rocks 4", "tree1": "Trees 1", "tree2": "Trees 2",
    "tree3": "Trees 3", "tree4": "Trees 4", "tree5": "Trees 5",
    "tree6": "Trees 6", "sand": "Sand", "snow": "Snow", "koke": "Moss",
    "ice": "Ice", "grass": "Grass",
}
KIND_NAMES = {"G": "Ground", "O": "Plate", "F": "Far", "B": "Back",
              "V": "Scenery", "T": "Terrain"}
# The kinds that the menu lets the player choose, with the name of their file.
CHOICE_KINDS = [("G", "ground"), ("F", "far"), ("B", "back"), ("O", "plate")]
# The pieces of the terrain moves (the field effects).
TERRAINS = {"grass": "Grassy", "mist": "Misty", "aqua": "Aqua", "ice": "Icy",
            "eleki": "Electric"}
# The kinds of pieces that are not signed "BG": a single 3D model.
SINGLE_KINDS = ("F", "B")


def read_file_list(path):
  """Reads the list of the files of the archive (a text file of the source of
  X and Y): returns {file number: name of the file}."""
  names = {}
  with open(path, encoding="latin1") as f:
    for line in f:
      match = re.match(r"\s*\w+?_((?:btl|fwaza)_[A-Za-z0-9_]+?)_(?:LZ_BIN|BCHMDL)"
                       r"\s*=\s*(\d+)", line)
      if match:
        names[int(match.group(2))] = match.group(1)
  return names


def stem_of_name(file_name):
  """btl_G_kusa gives ("G", "kusa"); btl_O_do_0_m gives ("O", "do_0")."""
  if file_name.startswith("fwaza_"):
    return ("T", file_name[len("fwaza_"):])
  match = re.match(r"btl_([A-Z])_(.+)$", file_name)
  if not match:
    return ("", "")
  kind, stem = match.group(1), match.group(2)
  # The plates end with _m.
  if kind == "O" and stem.endswith("_m"):
    stem = stem[:-2]
  return (kind, stem)


def stem_of_data(data):
  """The kind and the name of a file of ORAS: the shortest name of a model of
  the file that starts with btl_<kind>_ (the model of the piece itself)."""
  best = None
  for match in re.finditer(rb"btl_([A-Z])_([A-Za-z0-9_]+)", data):
    name = match.group(2).decode("latin1")
    if name.endswith("_t"):
      continue
    if best is None or len(name) < len(best[1]):
      best = (match.group(1).decode(), name)
  if best is None:
    # A piece without model names: a terrain piece.
    return ("T", "?")
  kind, name = best
  return (kind, name[:-2] if kind == "O" and name.endswith("_m") else name)


def english_name(kind, stem):
  return TERRAINS.get(stem, stem) if kind == "T" else STEMS.get(stem, stem)


def english_of(kind, stem):
  return "%s - %s" % (KIND_NAMES.get(kind, kind or "?"), english_name(kind, stem))


def write_choices(folder, names_folder, oras_pieces, xy_pieces):
  """Writes, for each kind of piece, the list of choices of the menu: the game
  default, the pieces of ORAS, then the pieces of X and Y.

  folder/choices_<kind>.bin: u16 entry count n; u16 file of each entry (the
  entry 0 is the default and has no file); u16 count of the files of ORAS of
  the kind; u16 each of these files. The files of ORAS are the ones that the
  game can ask for: the plugin replaces all of them by the chosen piece.
  names_folder/bg<name>.bin: the names of the entries (the format of sango/names).
  """
  for kind, name in CHOICE_KINDS:
    entries = [(0, "Game Default")]
    sources = []
    for number, (other_kind, stem) in sorted(oras_pieces.items()):
      if other_kind == kind:
        entries.append((number, "ORAS - %s" % english_name(kind, stem)))
        sources.append(number)
    for number, (other_kind, stem) in xy_pieces:
      if other_kind == kind:
        entries.append((number, "XY - %s" % english_name(kind, stem)))
    values = [len(entries)] + [e[0] for e in entries]
    values += [len(sources)] + sources
    with open(os.path.join(folder, "choices_%s.bin" % kind), "wb") as f:
      f.write(struct.pack("<%dH" % len(values), *values))
    header = 8 + 4 * len(entries)
    body = bytearray(b"\0")
    offsets = []
    for _, label in entries:
      offsets.append(header + len(body))
      body += label.encode("utf-8") + b"\0"
    with open(os.path.join(names_folder, "bg%s.bin" % name), "wb") as f:
      f.write(struct.pack("<II", header + len(body), len(entries)))
      f.write(struct.pack("<%dI" % len(offsets), *offsets) + bytes(body))


def read_file(parts):
  data = parts[0]
  if data[:1] == b"\x11":
    data = garc.decompress_lz11(data)
  return data


def kind_of(data):
  """Returns the kind of a piece (G, O, V, F, B), or "" when it has none."""
  kinds = re.findall(rb"btl_([A-Za-z])_", data)
  if not kinds:
    return ""
  if data[:2] == b"BG" and b"O" in kinds and b"G" not in kinds:
    return "O"
  for wanted in (b"G", b"O", b"V", b"F", b"B"):
    if wanted in kinds:
      return wanted.decode()
  return ""


def is_piece(data):
  """Returns true for a file that the tool exports."""
  if data[:2] == b"BG":
    return True
  return data[:4] == b"BCH\x00" and kind_of(data) in SINGLE_KINDS


def names_of(data):
  """Returns some names that a piece has inside (for the lists)."""
  found = []
  # The names of the models of the game first (btl_...), then any name.
  pattern = rb"btl_[A-Za-z0-9_]{3,}"
  if not re.search(pattern, data):
    pattern = rb"[A-Za-z][A-Za-z0-9_]{5,}"
  for match in re.finditer(pattern, data):
    name = match.group().decode("latin1")
    if name.startswith(("Default", "Private", "Bake", "Calc", "Gen", "BBox",
                        "Nw4c", "Fragment", "Constant", "Frag", "Vertex")):
      continue
    if name not in found:
      found.append(name)
    if len(found) == 4:
      break
  return found


def all_names(data):
  """Returns all the model names of a piece (a set)."""
  return set(m.group().decode("latin1") for m in
             re.finditer(rb"btl_[A-Za-z0-9_]{3,}|c[0-9]{2}[A-Za-z0-9_]{3,}",
                         data))


def find_match(kind, stem, oras_pieces):
  """Returns the number of the file of ORAS that has the same kind and the same
  name, or 0xFFFF."""
  for number, (other_kind, other_stem) in oras_pieces.items():
    if other_kind == kind and other_stem == stem:
      return number
  return 0xFFFF


def short_name(names):
  return ", ".join(n.replace("btl_", "") for n in names[:2]) or "no name"


def main():
  parser = argparse.ArgumentParser(
      description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument("xy_archive", help="The archive a/1/4/6 of X and Y.")
  parser.add_argument("out_dir")
  parser.add_argument("--spica", help="The folder of SPICA (has SPICA.dll).")
  parser.add_argument("--oras-archive",
                      help="The archive a/1/3/6 of ORAS. The tool finds, for "
                           "each piece of X and Y, the file of ORAS to replace.")
  parser.add_argument("--xy-list",
                      help="The list of the files of the archive of X and Y "
                           "(in its source). It gives the names of the files.")
  parser.add_argument("--no-convert", action="store_true",
                      help="Do not convert the format (for tests).")
  parser.add_argument("--fast", action="store_true",
                      help="Do not compress the files (bigger files, faster).")
  args = parser.parse_args()
  if not args.spica and not args.no_convert:
    sys.exit("Give the folder of SPICA with --spica.")

  xy = garc.read_garc(args.xy_archive)
  pieces = {}
  for index, parts in enumerate(xy):
    data = read_file(parts)
    if is_piece(data):
      pieces[index] = data

  # Each model of each piece is converted. The converter works on files.
  models = {}
  layout = {}
  for index, data in pieces.items():
    layout[index] = []
    if data[:2] == b"BG":
      count = struct.unpack_from("<H", data, 2)[0]
      offsets = struct.unpack_from("<%dI" % (count + 1), data, 4)
      chunks = [data[offsets[part]:offsets[part + 1]] for part in range(count)]
    else:
      chunks = [data]
    for part, chunk in enumerate(chunks):
      key = index * 16 + part
      layout[index].append(key)
      models[key] = chunk
  if not args.no_convert:
    print("Converting the 3D format of %d models..." % len(models), flush=True)
    convert_models(models, args.spica)

  oras_pieces = {}
  oras_lines = []
  if args.oras_archive:
    for number, parts in enumerate(garc.read_garc(args.oras_archive)):
      data = read_file(parts)
      if is_piece(data):
        kind, stem = stem_of_data(data)
        oras_pieces[number] = (kind, stem)
        oras_lines.append("%d: %s" % (number, english_of(kind, stem)))

  xy_names = read_file_list(args.xy_list) if args.xy_list else {}
  folder = os.path.join(args.out_dir, "battle_background")
  os.makedirs(folder, exist_ok=True)
  index_entries = []
  lines = []
  labels = []
  matches = []
  xy_pieces = []
  for rank, index in enumerate(sorted(pieces)):
    data = pieces[index]
    if data[:2] == b"BG":
      pack = garc.pack_mm([models[key] for key in layout[index]], magic=b"BG")
    else:
      pack = models[layout[index][0]]
    number = FIRST_FILE + rank
    packed = (garc.compress_lz11_store(pack) if args.fast
              else garc.compress_lz11(pack))
    with open(os.path.join(folder, "%d.bin" % number), "wb") as f:
      f.write(packed)
    index_entries.append((number, len(packed)))
    if index in xy_names:
      kind, stem = stem_of_name(xy_names[index])
    else:
      kind, stem = stem_of_data(data)
    match = find_match(kind, stem, oras_pieces)
    matches.append(match)
    xy_pieces.append((number, (kind, stem)))
    english = english_of(kind, stem)
    lines.append("%d (file %d of X and Y, replaces %s): %s" %
                 (number, index, "none" if match == 0xFFFF else match,
                  english))
    labels.append("%d: %s" % (number, english))
  with open(os.path.join(folder, "index.bin"), "wb") as f:
    f.write(struct.pack("<I", len(index_entries)))
    for entry in index_entries:
      f.write(struct.pack("<II", *entry))
  # The piece of ORAS that each new piece replaces (u16 each), and the list of
  # names for the menu (the format of sango/names).
  with open(os.path.join(folder, "match.bin"), "wb") as f:
    f.write(struct.pack("<%dH" % len(matches), *matches))
  header = 8 + 4 * len(labels)
  body = bytearray(b"\0")
  offsets = []
  for label in labels:
    offsets.append(header + len(body))
    body += label.encode("utf-8") + b"\0"
  names_folder = os.path.join(args.out_dir, "names")
  os.makedirs(names_folder, exist_ok=True)
  write_choices(folder, names_folder, oras_pieces, xy_pieces)
  with open(os.path.join(names_folder, "xybackground.bin"), "wb") as f:
    f.write(struct.pack("<II", header + len(body), len(labels)))
    f.write(struct.pack("<%dI" % len(offsets), *offsets) + bytes(body))
  if oras_lines:
    with open(os.path.join(folder, "oras_list.txt"), "w",
              encoding="utf-8") as f:
      f.write("\n".join(oras_lines) + "\n")
  with open(os.path.join(folder, "list.txt"), "w", encoding="utf-8") as f:
    f.write("\n".join(lines) + "\n")
  print("%d files written in %s" % (len(index_entries), folder))


if __name__ == "__main__":
  main()
