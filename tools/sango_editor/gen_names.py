"""Writes src/names.rs: the names of the models of the characters, of the
maps and of the items of Alpha Sapphire and Omega Ruby.

The names come from the plugin: lib/src/ui/page/enum_names.cc has the arrays
kModelNames and kMapNames. The tool takes the version of the arrays that the
game ORAS uses (the part inside `#ifndef GAME_XY`, not the part inside
`#ifdef GAME_XY`).

  python tools/sango_editor/gen_names.py
"""

import json
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
SOURCE = os.path.join(REPO, "lib", "src", "ui", "page", "enum_names.cc")
NAMES_JS = os.path.join(REPO, "tools", "maker_editor", "names.js")
OUT = os.path.join(HERE, "src", "names.rs")


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
    # The names of the items come from the names of the data files of the plugin.
    with open(NAMES_JS, encoding="utf-8") as f:
        text = f.read()
    items = json.loads(text[text.index("{"):text.rindex("}") + 1])["item"]
    out.append(rust_array("ITEM_NAMES", items))
    out.append("")
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))
    print("models: %d, maps: %d, items: %d" % (len(arrays["kModelNames"]), len(arrays["kMapNames"]), len(items)))


if __name__ == "__main__":
    main()
