"""Takes the battle models of the trainers of Pokemon X/Y for ORAS.

ORAS shows a 3D model for some trainers in battle. It already has the models
of Serena, Calem, Malva, Holocaster, Shauna, Tierno and Trevor. This tool reads
the other models in the archive of X or Y and writes them for the SD card. The
plugin then shows them with battle::TrainerModelExtension.

  python tools/export_xy_trainer_models.py <xy_a_1_4_0> <out_dir> \
      --spica <folder of SPICA>

The first argument is the archive of the battle models of the trainers of X or
Y: the file "a/1/4/0" of the RomFS. Each model has 5 files: the 3D model, the
base pack and three packs of motions. The tool writes:

  <out_dir>/trainer_model/index.bin   the list of the files
  <out_dir>/trainer_model/<n>.bin     one file (n is a number of the table)

Copy the folder "trainer_model" to sdmc:/sango/. The models use the numbers 75
and after, which are the numbers after the last file of the archive of ORAS.
The order of the models is the order of battle::XyTrainerModelId.

The 3D models of X and Y have an older format than the models of ORAS. The tool
converts them with the library SPICA.dll of the program SPICA (Windows only, see
tools/convert_bch.ps1). The option --spica gives its folder. The packs of motions
keep the format of X and Y: ORAS reads them without a change.

These files are data of the game: do not share them.
"""

import argparse
import os
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import garc

# The number of the first file of the models in the archive of ORAS (it has
# 75 files).
FIRST_FILE = 75
FILES_PER_MODEL = 5
# The number of the first file of the packs of battle motions of the models of
# ORAS, with the motions that the battle asks for (see fill_empty_motions). The
# pack of the model number n is the file FIRST_ORAS_MOTION_FILE + n. The files
# of the models of X and Y end at 139.
FIRST_ORAS_MOTION_FILE = 140
ORAS_MODEL_COUNT = 15

# (name of battle::XyTrainerModelId, number of the model in the archive of X
# and Y). The archive of X and Y has the models in the order of the list of
# the graphics: the hero and the heroine, the friends, the professor, the
# leaders of the teams...
MODELS = [
    ("Sycamore", 5),
    ("Lysandre", 7),
    ("FlareAdminMale", 8),
    ("FlareAdminFemale", 9),
    ("Aliana", 10),
    ("Xerosic", 11),
    ("FlareGruntMale", 12),
    ("FlareGruntFemale", 13),
    ("Celosia", 14),
    ("Bryony", 15),
    ("Mable", 16),
    ("Az", 17),
    ("LysandreFinal", 18),
]


def convert_models(files, spica_dir):
  """Converts the 3D models (the files that start with BCH) to the format of
  ORAS. `files` is {key: bytes}. The function changes the values in place."""
  script = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        "convert_bch.ps1")
  with tempfile.TemporaryDirectory() as temp:
    source = os.path.join(temp, "in")
    target = os.path.join(temp, "out")
    os.makedirs(source)
    for key, data in files.items():
      if data[:4] == b"BCH\x00":
        with open(os.path.join(source, "%d.bch" % key), "wb") as f:
          f.write(data)
    result = subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
         script, "-SpicaDir", spica_dir, "-InDir", source, "-OutDir", target],
        capture_output=True, text=True)
    if result.returncode != 0 or "FAILED" in result.stdout:
      sys.exit("The conversion failed:\n" + result.stdout + result.stderr)
    for key in files:
      path = os.path.join(target, "%d.bch" % key)
      if os.path.isfile(path):
        files[key] = open(path, "rb").read()


# The slot of the battle motions that the battle of ORAS plays when the player
# throws a Poké Ball, and the slots with the throw of the trainers of X and Y
# (the first slot that has a motion).
PLAYER_THROW_SLOT = 6
TRAINER_THROW_SLOTS = (2, 1)
# The battle motions of the games: the slots 0 to 15 are the motions
# of the body, the slots after them are the motions of the mouth, which a model
# of X and Y does not have. The sequences of the Mega Evolution of the player
# play the slot 15 (the slot 14 is the beginning of the motion).
BODY_SLOT_COUNT = 16
MEGA_SLOTS = (14, 15, 16)
# The motion of the model that the Mega Evolution plays: its pose (slot 4).
MEGA_OWN_SLOT = 4
# The model of ORAS that gives the Mega Evolution: Brendan, the file of the
# battle motions (3 files after the model and its base).
DONOR_MODEL = 5


def read_motion(pack, slot):
  """Returns the motion of a slot of a pack of battle motions, or None."""
  count = struct.unpack_from("<H", pack, 2)[0]
  offsets = struct.unpack_from("<%dI" % (count + 1), pack, 4)
  section = pack[offsets[0]:offsets[1]]
  slots = struct.unpack_from("<I", section, 0)[0]
  table = struct.unpack_from("<%dI" % slots, section, 4)
  end = struct.unpack_from("<I", section, 4 + 4 * slots)[0]
  if slot >= slots or table[slot] == 0:
    return None
  later = [x for x in sorted(table) + [end] if x > table[slot]]
  return section[table[slot]:min(later)]


def fill_empty_motions(pack, mega=None):
  """Gives a pack of battle motions the motions that ORAS asks for.

  The pack is a container ("BS") of three parts. The first part is the section
  of the motions: the number of slots, one offset for each slot (0 is no
  motion), the end of the data, then the motions, one after the other in the
  order of the slots. A trainer of X and Y has only 4 motions, but the battle
  of ORAS asks for more (the throw of the player is the slot 6, the Mega
  Evolution the slot 15...). A slot without motion leaves the model in T-pose.
  The function gives each empty slot of the body the throw of the trainer
  (slot 2, or 1), and the slots of the Mega Evolution the pose of the model
  (slot 4), or the motion `mega` of the hero of ORAS when the model has no pose.
  It keeps the order of the offsets and does not fill the slots of the mouth.
  """
  count = struct.unpack_from("<H", pack, 2)[0]
  offsets = struct.unpack_from("<%dI" % (count + 1), pack, 4)
  parts = [pack[offsets[i]:offsets[i + 1]] for i in range(count)]
  section = parts[0]
  if len(section) < 8:
    return pack
  slots = struct.unpack_from("<I", section, 0)[0]
  if slots < BODY_SLOT_COUNT:
    return pack
  table = list(struct.unpack_from("<%dI" % slots, section, 4))
  end = struct.unpack_from("<I", section, 4 + 4 * slots)[0]
  source = [slot for slot in TRAINER_THROW_SLOTS if table[slot] != 0]
  if not source:
    return pack
  starts = sorted(offset for offset in table if offset)

  def motion_of(slot):
    later = [offset for offset in starts + [end] if offset > table[slot]]
    return section[table[slot]:min(later)]

  throw = motion_of(source[0])
  head = 4 + 4 * slots + 4
  data = bytearray()
  new_table = []
  for slot in range(slots):
    if table[slot] != 0:
      new_table.append(head + len(data))
      data += motion_of(slot)
    elif slot in MEGA_SLOTS and (table[MEGA_OWN_SLOT] != 0 or mega is not None):
      new_table.append(head + len(data))
      data += motion_of(MEGA_OWN_SLOT) if table[MEGA_OWN_SLOT] != 0 else mega
    elif slot < BODY_SLOT_COUNT:
      new_table.append(head + len(data))
      data += throw
    else:
      new_table.append(0)
  new_section = (struct.pack("<I", slots) +
                 struct.pack("<%dI" % slots, *new_table) +
                 struct.pack("<I", head + len(data)) + bytes(data))
  parts[0] = new_section
  return garc.pack_mm(parts, magic=b"BS")


def main():
  parser = argparse.ArgumentParser(
      description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument("xy_archive", nargs="?")
  parser.add_argument("out_dir", nargs="?")
  parser.add_argument("--spica", help="The folder of SPICA (has SPICA.dll).")
  parser.add_argument("--oras-archive",
                      help="The archive of the battle models of the trainers "
                           "of ORAS (a/1/3/3). It gives the motion of the "
                           "Mega Evolution of the player.")
  parser.add_argument("--no-convert", action="store_true",
                      help="Do not convert the format (for tests).")
  parser.add_argument("--fast", action="store_true",
                      help="Do not compress the files (bigger files, faster).")
  parser.add_argument("--list", action="store_true",
                      help="Show the models, then stop.")
  args = parser.parse_args()

  if args.list:
    for index, (name, model) in enumerate(MODELS):
      first = FIRST_FILE + index * FILES_PER_MODEL
      print("%2d  %-18s  model %2d of X and Y  files %d to %d" %
            (index, name, model, first, first + FILES_PER_MODEL - 1))
    return
  if not args.xy_archive or not args.out_dir:
    parser.error("give the archive and the output folder")
  if not args.spica and not args.no_convert:
    sys.exit("Give the folder of SPICA with --spica.")

  xy = garc.read_garc(args.xy_archive)
  if len(xy) < (MODELS[-1][1] + 1) * FILES_PER_MODEL:
    sys.exit("The archive has %d files: it is not the archive of the trainer "
             "models of X and Y (a/1/4/0)." % len(xy))

  # The files of X and Y, by the number that they will have for ORAS.
  mega = None
  if args.oras_archive:
    oras = garc.read_garc(args.oras_archive)
    donor = oras[DONOR_MODEL * FILES_PER_MODEL + 2][0]
    if donor[:1] == b"":
      donor = garc.decompress_lz11(donor)
    mega = read_motion(donor, MEGA_SLOTS[-1])
  files = {}
  for index, (name, model) in enumerate(MODELS):
    for part in range(FILES_PER_MODEL):
      source = xy[model * FILES_PER_MODEL + part][0]
      if part == 2:  # The pack of battle motions.
        source = fill_empty_motions(source, mega)
      files[FIRST_FILE + index * FILES_PER_MODEL + part] = source
  if not args.no_convert:
    print("Converting the 3D format of %d models..." % len(MODELS), flush=True)
    convert_models(files, args.spica)

  if args.oras_archive:
    # A model of ORAS can be the model of the player: its pack of motions has
    # the same holes (the Mega Evolution, the throw...) as the ones of X and Y.
    for model in range(ORAS_MODEL_COUNT):
      pack = oras[model * FILES_PER_MODEL + 2][0]
      if pack[:1] == b"":
        pack = garc.decompress_lz11(pack)
      if len(pack) < 8:
        continue
      filled = fill_empty_motions(pack, mega)
      if filled != pack:
        files[FIRST_ORAS_MOTION_FILE + model] = filled
  folder = os.path.join(args.out_dir, "trainer_model")
  os.makedirs(folder, exist_ok=True)
  index = []
  for number in sorted(files):
    data = files[number]
    # The files of the archive of ORAS are compressed files.
    packed = (garc.compress_lz11_store(data) if args.fast
              else garc.compress_lz11(data))
    with open(os.path.join(folder, "%d.bin" % number), "wb") as f:
      f.write(packed)
    index.append((number, len(packed)))
  for model_index, (name, model) in enumerate(MODELS):
    first = FIRST_FILE + model_index * FILES_PER_MODEL
    size = sum(entry[1] for entry in index
               if first <= entry[0] < first + FILES_PER_MODEL)
    print("%2d %-18s %9d bytes" % (model_index, name, size), flush=True)
  with open(os.path.join(folder, "index.bin"), "wb") as f:
    f.write(struct.pack("<I", len(index)))
    for entry in index:
      f.write(struct.pack("<II", *entry))
  print("%d files written in %s" % (len(index), folder))


if __name__ == "__main__":
  main()
