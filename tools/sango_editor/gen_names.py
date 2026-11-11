"""Writes src/names.rs: the names of the models of the characters, of the
maps and of the items of Alpha Sapphire and Omega Ruby.

The names come from the plugin: lib/src/ui/page/enum_names.cc has the arrays
kModelNames and kMapNames. The tool takes the version of the arrays that the
game ORAS uses (the part inside `#ifndef GAME_XY`, not the part inside
`#ifdef GAME_XY`).

  python tools/sango_editor/gen_names.py
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
SOURCE = os.path.join(REPO, "lib", "src", "ui", "page", "enum_names.cc")
OUT = os.path.join(HERE, "src", "names.rs")

sys.path.insert(0, os.path.join(REPO, "tools"))
import gen_data_names


def item_names():
    """The readable names of the items, in the order of their numbers.

    The names come from the item enum of the library. The empty and the unknown
    values are skipped, so the number of an item is its index plus one.
    """
    seen = set()
    names = []
    enum = ("lib/include/pokemon/constant/item.h", "ItemId")
    for name, _ in gen_data_names.read_names(*enum):
        readable = gen_data_names.pretty(name)
        if (readable in ("None", "XXX", "YYY") or readable.startswith("Unknow")
                or gen_data_names.normalize(readable) in seen):
            continue
        seen.add(gen_data_names.normalize(readable))
        names.append(readable)
    return names


def read_arrays(path):
    """Returns {array name: [names]} for the game ORAS."""
    arrays = {}
    inside_xy = False  # inside `#ifdef GAME_XY`
    inside_not_xy = False  # inside `#ifndef GAME_XY`
    current = None
    with open(path, encoding="utf-8") as f:
        for line in f:
            text = line.strip()
            if text.startswith("#ifdef GAME_XY"):
                inside_xy = True
                continue
            if text.startswith("#ifndef GAME_XY"):
                inside_not_xy = True
                continue
            if text.startswith("#endif"):
                inside_xy = inside_not_xy = False
                continue
            if inside_xy:
                continue
            match = re.match(r"const c8\* (\w+)\[(\d+)\] = \{", text)
            if match:
                current = match.group(1)
                arrays[current] = []
                continue
            if current and text.startswith("};"):
                current = None
                continue
            if current:
                m = re.match(r'"(.*)",?$', text)
                if m:
                    arrays[current].append(m.group(1).replace('\\"', '"'))
    return arrays


def rust_array(name, values):
    lines = ["pub const %s: [&str; %d] = [" % (name, len(values))]
    for v in values:
        lines.append('    "%s",' % v.replace("\\", "\\\\").replace('"', '\\"'))
    lines.append("];")
    return "\n".join(lines)


def main():
    arrays = read_arrays(SOURCE)
    out = ["//! The names of the models of the characters, of the maps and of the items.", "//!",
           "//! Written by `gen_names.py` from the plugin: do not change it by hand.", ""]
    out.append(rust_array("MODEL_NAMES", arrays["kModelNames"]))
    out.append("")
    out.append(rust_array("MAP_NAMES", arrays["kMapNames"]))
    out.append("")
    # The names of the items come from the item enum of the library.
    items = item_names()
    out.append(rust_array("ITEM_NAMES", items))
    out.append("")
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))
    print("models: %d, maps: %d, items: %d" % (len(arrays["kModelNames"]), len(arrays["kMapNames"]), len(items)))


if __name__ == "__main__":
    main()
