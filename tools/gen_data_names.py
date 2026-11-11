#!/usr/bin/env python3
"""Writes the lists of names that the menus of the plugin load (core::NameFile).

This tool reads the enums of the library and writes one file for each kind of
name (species.bin, move.bin...) in the given folder. The plugin reads the
files from sdmc:/sango/names.

  python tools/gen_data_names.py --names <folder>

Run it again when an enum changes.
"""

import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# (array name, header, enum name, game). The game is "XY", "ORAS" or None.
ENUMS = [
    ("kSpeciesNames", "lib/include/pokemon/constant/species.h", "SpeciesId", None),
    ("kMoveNames", "lib/include/pokemon/constant/move.h", "MoveId", None),
    ("kItemNames", "lib/include/pokemon/constant/item.h", "ItemId", None),
    ("kAbilityNames", "lib/include/pokemon/constant/ability.h", "AbilityId", None),
    ("kNatureNames", "lib/include/pokemon/constant/nature.h", "Nature", None),
    ("kTypeNames", "lib/include/pokemon/constant/type.h", "TypeId", None),
    ("kFormNames", "lib/include/pokemon/constant/form.h", "FormId", None),
    ("kTrainerNames", "lib/include/battle/constant/trainer.h", "TrainerId", None),
    ("kXyTrainerModelNames", "lib/include/battle/constant/xy_trainer_model.h",
     "XyTrainerModelId", None),
    ("kTrainerClassNames", "lib/include/battle/constant/trainer_class.h",
     "TrainerClassId", None),
    ("kModelNames", "lib/include/overworld/constant/model.h", "ModelId", None),
    ("kEvolutionMethodNames", "lib/include/pokemon/constant/evolution_method.h",
     "EvolutionMethod", None),
    ("kEncounterMethodNames", "lib/include/overworld/constant/encounter_method.h",
     "EncounterMethod", None),
    ("kFormatNames", "lib/include/battle/constant/format.h", "Format", None),
    ("kMusicNames", "lib/include/system/constant/background_music_xy.h",
     "BackgroundMusicId", "XY"),
    ("kMusicNames", "lib/include/system/constant/background_music.h",
     "BackgroundMusicId", "ORAS"),
    ("kMapNames", "lib/include/overworld/constant/map.h", "MapId", "XY"),
    ("kMapNames", "lib/include/overworld/constant/map.h", "MapId", "ORAS"),
]


ITEM = re.compile(r"^\s*k(\w+)\s*(?:=\s*(.+?))?\s*$", re.S)


def read_names(path, enum_name, game=None):
    """Returns all the names of an enum, also the names with the same value
    (the forms of the species use the same numbers): [(name, value)]."""
    text = open(os.path.join(REPO, path), encoding="utf-8").read()
    # A header with two versions of an enum (XY first, then ORAS).
    find = text.rindex if game == "ORAS" else text.index
    start = find("enum class %s" % enum_name)
    body = text[text.index("{", start) + 1:text.index("};", start)]
    # Remove the comments and the lines of the preprocessor, then cut at the
    # commas: an enum can have several values on one line.
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    body = re.sub(r"//[^\n]*", "", body)
    body = re.sub(r"^\s*#[^\n]*", "", body, flags=re.M)
    names = []
    known = {}
    next_value = 0
    for item in body.split(","):
        match = ITEM.match(item)
        if not match:
            continue
        name, value = match.groups()
        if name in ("Count", "Max"):
            continue
        if value is not None:
            value = value.strip()
            if value.startswith("k") and value[1:] in known:
                # An alias: kFirstFree = kLastValue.
                next_value = known[value[1:]]
            else:
                try:
                    next_value = int(value, 0)
                except ValueError:
                    # GAME_CONSTANT(...) or an expression: skip the name.
                    continue
        known[name] = next_value
        names.append((name, next_value))
        next_value += 1
    return names


def entry_names(array, path, enum_name, game):
    """The names of an entry of ENUMS. The models of trainers for trainers.txt
    are the 13 models of X and Y (the values 0 to 12), then the 15 models of
    Alpha Sapphire (the values 13 to 27)."""
    names = read_names(path, enum_name, game)
    if array == "kXyTrainerModelNames" and game != "XY":
        for name, value in read_names(
                "lib/include/battle/constant/trainer_model.h",
                "TrainerModelId", "ORAS"):
            names.append((name, value + 13))
    return names


def normalize(name):
    return re.sub(r"[^a-z0-9]", "", name.lower())


# The name of the file of each enum.
NAME_KINDS = {
    "kSpeciesNames": "species", "kMoveNames": "move", "kItemNames": "item",
    "kAbilityNames": "ability", "kNatureNames": "nature", "kTypeNames": "type",
    "kFormNames": "form", "kTrainerNames": "trainer", "kModelNames": "model",
    "kXyTrainerModelNames": "trainermodel",
    "kTrainerClassNames": "trainerclass",
    "kEvolutionMethodNames": "evolution", "kEncounterMethodNames": "method",
    "kFormatNames": "format", "kMusicNames": "music", "kMapNames": "map",
}


def pretty(name):
    words = re.findall(r"[A-Z][a-z]+|[A-Z]+(?![a-z])|\d+|[a-z]+", name)
    return " ".join(words)


NAME_FOLDER_KINDS_SKIPPED = ("form",)


def write_name_files(folder):
    """Writes the lists of names that the menus of the plugin load.

    One file for each kind of name (species.bin, move.bin...), for Alpha
    Sapphire. The file has the size of the file, the number of names, one
    offset for each name (from the start of the file), then the names: the
    plugin reads the file, adds the address of the file to the offsets, and
    gives the result to WithArray(). The index of a name is its value.
    """
    import struct
    os.makedirs(folder, exist_ok=True)
    count_written = 0
    for array, path, enum, game in ENUMS:
        if game == "XY":
            continue
        kind = NAME_KINDS[array]
        if kind in NAME_FOLDER_KINDS_SKIPPED:
            continue
        values = {}
        for name, value in entry_names(array, path, enum, game):
            readable = pretty(name)
            if readable in ("XXX", "YYY") or readable.startswith("Unknow"):
                continue
            if value < 20000:
                values.setdefault(value, readable)
        if not values:
            continue
        count = max(values) + 1
        if count > 20000:
            continue
        header = 8 + 4 * count
        body = bytearray(b"\0")  # The empty name.
        offsets = []
        for value in range(count):
            text = values.get(value, "")
            if text == "":
                offsets.append(header)
                continue
            offsets.append(header + len(body))
            body += text.encode("utf-8") + b"\0"
        data = struct.pack("<II", header + len(body), count)
        data += struct.pack("<%dI" % count, *offsets) + bytes(body)
        with open(os.path.join(folder, kind + ".bin"), "wb") as f:
            f.write(data)
        count_written += 1
    print("wrote %d name files in %s" % (count_written, folder))


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--names":
        write_name_files(sys.argv[2])
    else:
        sys.exit("Usage: gen_data_names.py --names <folder>")
