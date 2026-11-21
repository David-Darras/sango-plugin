"""Writes the lists of the layout archives that the plugin can replace from the SD card.

The plugin reads `sdmc:/sango-studio/layout/<archive>/<file>.bin` in place of a file of an archive that has layouts (the menus of
the game). This script looks in a dump of the romfs for the archives that have layouts, and writes two files that
`lib/src/core/archive.cc` includes:

  lib/include/core/data/layout_pool.inc   the number of entries of the shared pool
  lib/include/core/data/layout_sets.inc   one row of the table of the archives for each archive that has layouts

Run it again when the dump changes (it does not):

  python tools/gen_layout_sets.py <folder of the romfs dump>
"""

import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
import garc

# The biggest archive that is searched (the archives of the models and of the maps have no layout).
BIGGEST = 60_000_000
# The number of files of an archive that Studio can replace at most.
MAX_FILES = 64


def is_bundle(data):
  """True when the file is a layout bundle: a list of resource names, then a pane table and a DARC archive."""
  try:
    if data[0] in (0x10, 0x11):
      data = garc.decompress_lz11(data)
    count = struct.unpack_from("<I", data, 0)[0]
    if not 0 < count < 400:
      return False
    name = data[4:68].split(b"\0")[0]
    if not name.endswith((b".bclan", b".bclyt", b".bclim", b".bcfnt")):
      return False
    position = 4 + count * 64
    panes = struct.unpack_from("<I", data, position)[0]
    position = (position + 4 + panes * 32 + 127) // 128 * 128
    if data[position:position + 4] != b"darc":
      return False
    # Only the bundles that hold a layout (a file `.bclyt`) matter.
    return b".bclyt" in data[4:4 + count * 64]
  except (IndexError, struct.error, ValueError):
    return False


def main(dump):
  root = os.path.join(dump, "a")
  rows = []
  for number in range(300):
    path = os.path.join(root, str(number // 100), str(number // 10 % 10), str(number % 10))
    if not os.path.isfile(path) or os.path.getsize(path) > BIGGEST:
      continue
    try:
      files = garc.read_garc(path)
    except ValueError:
      continue
    count = sum(1 for parts in files if any(is_bundle(d) for d in parts if len(d) > 8))
    if count:
      rows.append((number, min(count, MAX_FILES)))

  total = sum(capacity for _, capacity in rows)
  here = os.path.dirname(os.path.abspath(__file__))
  data = os.path.join(here, "..", "lib", "include", "core", "data")
  os.makedirs(data, exist_ok=True)
  header = "// Made by tools/gen_layout_sets.py from a dump of the romfs. Do not edit.\n"
  with open(os.path.join(data, "layout_pool.inc"), "w", newline="\n") as out:
    out.write(header)
    out.write("static constexpr u32 kLayoutPoolSize = %d;\n" % total)
  with open(os.path.join(data, "layout_sets.inc"), "w", newline="\n") as out:
    out.write(header)
    place = 0
    for number, capacity in rows:
      out.write(
          '{static_cast<ArchiveId>(%d), u"sdmc:/sango-studio/layout/%d/index.bin",\n'
          ' u"sdmc:/sango-studio/layout/%d/%%d.bin", kLayoutToken, true,\n'
          " layout_entries + %d, %d, 0, false},\n" % (number, number, number, place, capacity))
      place += capacity
  print("%d archives, %d entries" % (len(rows), total))


if __name__ == "__main__":
  main(sys.argv[1])
