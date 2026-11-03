"""Lists all the trainers of the game: number, class, name, format, maps and
team.

The data comes from the archives of the game (TrData, TrPokeData, the texts
and the zone data, which holds the characters of each map). Give the folder
that has them:

  python tools/export_trainers.py <folder of the archives> [output file]
      [--pkpawn <pkpawn-local>]

With --pkpawn, the tool also reads the scripts of the maps (it decompiles them
with pkpawn-local) to find the trainers that scripts start: rivals, leaders and
the Elite Four.

Without an output file, the list goes to the screen. The list is for you:
keep it in private/, not in the repository.
"""

import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import garc

# The texts of the game.
TEXT_SPECIES = 98
TEXT_TRAINER_CLASS = 21
TEXT_TRAINER_NAME = 22

FORMATS = ["Single", "Double", "Triple", "Rotation", "Horde"]


def read_lines(data):
  """Decodes a text file of the game: a list of lines."""
  _, count, _, _, section = struct.unpack_from("<HHIII", data, 0)
  lines = []
  key = 0x7C89
  base = section + 4
  for i in range(count):
    offset, length, _ = struct.unpack_from("<iHH", data, base + i * 8)
    line_key = key
    codes = []
    for j in range(length):
      code = struct.unpack_from("<H", data, section + offset + j * 2)[0]
      codes.append(code ^ line_key)
      line_key = ((line_key << 3) | (line_key >> 13)) & 0xFFFF
    key = (key + 0x2983) & 0xFFFF
    lines.append(to_text(codes))
  return lines


def to_text(codes):
  text = ""
  i = 0
  while i < len(codes) and codes[i] != 0:
    if codes[i] == 0x10:  # a tag: 0x10, the number of words, the words
      i += 2 + codes[i + 1]
      continue
    text += {0xE08E: "♂", 0xE08F: "♀"}.get(codes[i], chr(codes[i]))
    i += 1
  return text


def read_constants(header):
  """Returns {number: name of the constant} of battle/constant/trainer.h."""
  names = {}
  if not os.path.exists(header):
    return names
  for line in open(header, encoding="utf-8"):
    match = re.match(r"\s*k(\w+)\s*=\s*(\d+),", line)
    if match:
      names[int(match.group(2))] = "k" + match.group(1)
  return names


def read_maps(header):
  """Returns {number: name of the constant} of the ORAS maps of map.h."""
  names = {}
  if not os.path.exists(header):
    return names
  in_oras = False
  for line in open(header, encoding="utf-8"):
    if line.startswith("#else"):
      in_oras = True
    elif line.startswith("#endif"):
      in_oras = False
    match = re.match(r"\s*k(\w+)\s*=\s*(\d+),", line)
    if in_oras and match:
      names.setdefault(int(match.group(2)), "k" + match.group(1))
  return names


def read_places(zones):
  """Returns {trainer: [(map, local id)]} of the characters of the maps.

  A map file is a pack ("ZO") whose second part holds the events: a header
  (size, signs, characters, warps, triggers), then the signs of 20 bytes, then
  the characters of 48 bytes. The script of a character is 3000 + the number
  of the trainer when the character is a trainer of the game.
  """
  places = {}
  for zone, parts in enumerate(zones):
    data = parts[0]
    if data[0] == 0x11:
      data = garc.decompress_lz11(data)
    if data[:2] != b"ZO":
      continue
    count = struct.unpack_from("<H", data, 2)[0]
    offsets = struct.unpack_from("<%dI" % (count + 1), data, 4)
    events = data[offsets[1]:offsets[2]]
    if len(events) < 8:
      continue
    signs, characters = struct.unpack_from("<BB", events, 4)
    first = 8 + signs * 20
    for i in range(characters):
      record = first + i * 48
      local_id = struct.unpack_from("<H", events, record + 4)[0]
      script = struct.unpack_from("<H", events, record + 14)[0]
      if 3000 <= script < 3000 + 1000:
        places.setdefault(script - 3000, []).append((zone, local_id))
  return places


def read_rematches(sango):
  """Reads the rematch table of the game from the source folder.

  Each row is a trainer that can battle again: a map and ten trainer numbers,
  one for each level of rematch. Returns ({rematch: (base, step)},
  {base: map}): step 1 is the first rematch, the base is the first number.
  """
  def read(path):
    with open(path, encoding="cp932", errors="replace") as f:
      return f.read()

  trainers = {
      name: int(number) for name, number in re.findall(
          r"\b(TRID_\w+)\s*=\s*(\d+)", read(os.path.join(
              sango, "include", "assets", "trainer", "trid_def.h")))}
  zones = {
      name: int(number) for name, number in re.findall(
          r"#define\s+(ZONE_ID_\w+)\s*\(\s*(\d+)\s*\)", read(os.path.join(
              sango, "include", "assets", "field", "zonedata", "zone_id.h")))}
  table = read(os.path.join(sango, "include", "assets", "field", "trainereye",
                            "TrainerEye.cdat"))
  rematches = {}
  base_maps = {}
  for row in re.finditer(r"\{TRAINER_MODE_\w+,[^{]*?(ZONE_ID_\w+),[^{]*"
                         r"\{([^}]*)\}", table):
    numbers = [trainers.get(n) for n in re.findall(r"trainer::(TRID_\w+)",
                                                   row.group(2))]
    if not numbers or numbers[0] is None:
      continue
    base = numbers[0]
    if row.group(1) in zones:
      base_maps[base] = zones[row.group(1)]
    seen = {base}
    for number in numbers[1:]:
      if number is None or number in seen or number in rematches:
        continue
      seen.add(number)
      rematches[number] = (base, len(seen) - 1)
  return rematches, base_maps


def split_arguments(text):
  """Splits the text of the arguments of a call at the top-level commas."""
  arguments = []
  depth = 0
  current = ""
  for char in text:
    if char in "([":
      depth += 1
    elif char in ")]":
      depth -= 1
    if char == "," and depth == 0:
      arguments.append(current.strip())
      current = ""
    else:
      current += char
  if current.strip():
    arguments.append(current.strip())
  return arguments


def read_battle_numbers(source):
  """Returns the numbers that a decompiled script gives to a trainer battle.

  A battle starts with the native start_trainer_battle(trainer, second
  trainer, ...). Scripts call it through small functions, so the numbers are
  the literal arguments of the calls to these functions.
  """
  numbers = set()
  wrappers = {}  # name -> indexes of the parameters that are trainers
  for match in re.finditer(r"^(\w+)\(([^)]*)\)\s*\{(.*?)^\}", source,
                           re.M | re.S):
    name, parameters, body = match.groups()
    parameters = [p.strip() for p in parameters.split(",") if p.strip()]
    for call in re.finditer(r"start_trainer_battle\(([^;]*)\);", body):
      for argument in split_arguments(call.group(1))[:2]:
        if argument in parameters:
          wrappers.setdefault(name, set()).add(parameters.index(argument))
  for call in re.finditer(r"start_trainer_battle\(([^;]*)\);", source):
    for argument in split_arguments(call.group(1))[:2]:
      if re.fullmatch(r"(0x[0-9a-f]+|\d+)", argument):
        numbers.add(int(argument, 0))
  for name, indexes in wrappers.items():
    for call in re.finditer(r"\b%s\(([^;]*)\);" % re.escape(name), source):
      arguments = split_arguments(call.group(1))
      for index in indexes:
        if index < len(arguments) and re.fullmatch(
            r"(0x[0-9a-f]+|\d+)", arguments[index]):
          numbers.add(int(arguments[index], 0))
  return {n for n in numbers if 0 < n < 1000}


def read_script_places(zones, pkpawn):
  """Returns {trainer: [(map, "script")]} of the battles in the map scripts.

  The scripts of a map are the second part of the events (after the
  characters) and the third part of the pack. pkpawn-local reads them.
  """
  import subprocess
  import tempfile
  places = {}
  with tempfile.TemporaryDirectory() as work:
    for zone, parts in enumerate(zones):
      data = parts[0]
      if data[0] == 0x11:
        data = garc.decompress_lz11(data)
      if data[:2] != b"ZO":
        continue
      count = struct.unpack_from("<H", data, 2)[0]
      offsets = struct.unpack_from("<%dI" % (count + 1), data, 4)
      events = data[offsets[1]:offsets[2]]
      scripts = {}
      if len(events) > 8:
        size = struct.unpack_from("<I", events, 0)[0]
        if len(events) > size + 8:
          scripts["events"] = events[size + 4:]
      if offsets[3] - offsets[2] > 8:
        scripts["map"] = data[offsets[2]:offsets[3]]
      for kind, script in scripts.items():
        source_path = os.path.join(work, "%d_%s.amx" % (zone, kind))
        text_path = source_path[:-4] + ".p"
        with open(source_path, "wb") as f:
          f.write(script)
        result = subprocess.run([pkpawn, "decompile", source_path, text_path],
                                capture_output=True)
        if result.returncode != 0 or not os.path.exists(text_path):
          continue
        with open(text_path, encoding="utf-8", errors="replace") as f:
          for number in read_battle_numbers(f.read()):
            places.setdefault(number, []).append((zone, kind))
  return places


def read_team(data, count, kind):
  """Returns [(species, level, form)] of the data of a team."""
  if count == 0:
    return []
  size = len(data) // count
  team = []
  for i in range(count):
    level, species, form = struct.unpack_from("<HHH", data, i * size + 2)
    team.append((species, level, form))
  return team


def main():
  arguments = sys.argv[1:]
  pkpawn = None
  if "--pkpawn" in arguments:
    at = arguments.index("--pkpawn")
    pkpawn = arguments[at + 1]
    del arguments[at:at + 2]
  sango = None
  if "--sango" in arguments:
    at = arguments.index("--sango")
    sango = arguments[at + 1]
    del arguments[at:at + 2]
  if len(arguments) < 1:
    print(__doc__)
    return 1
  folder = arguments[0]
  trainers = garc.read_garc(os.path.join(folder, "TrData.garc"))
  teams = garc.read_garc(os.path.join(folder, "TrPokeData.garc"))
  texts = garc.read_garc(os.path.join(folder, "message_English.garc"))
  species = read_lines(texts[TEXT_SPECIES][0])
  classes = read_lines(texts[TEXT_TRAINER_CLASS][0])
  names = read_lines(texts[TEXT_TRAINER_NAME][0])
  here = os.path.dirname(os.path.abspath(__file__))
  constants = read_constants(
      os.path.join(here, "..", "lib", "include", "battle", "constant",
                   "trainer.h"))

  maps = read_maps(
      os.path.join(here, "..", "lib", "include", "overworld", "constant",
                   "map.h"))
  zones = garc.read_garc(os.path.join(folder, "zonedata_y.garc"))
  places = read_places(zones)
  if pkpawn:
    # The battles that scripts start: rivals, leaders, the Elite Four...
    for number, found in read_script_places(zones, pkpawn).items():
      known = {zone for zone, _ in places.get(number, [])}
      for zone, _ in found:
        if zone not in known:
          places.setdefault(number, []).append((zone, 0))
          known.add(zone)

  # The first six trainers (the rival Brendan and May) are on Route 102.
  for number in range(1, 7):
    places.setdefault(number, [(24, 0)])

  rematches = {}
  if sango:
    rematches, base_maps = read_rematches(sango)
    for base, zone in base_maps.items():
      places.setdefault(base, [(zone, 0)])
    # A rematch is on the map of the trainer that it replaces.
    for number, (base, _) in rematches.items():
      places[number] = places.get(base, [])

  out = []
  out.append("# All the trainers of the game, by number.")
  out.append("# number | constant | class | name | format | maps | team "
             "(species level)")
  out.append("")
  for number, parts in enumerate(trainers):
    data = parts[0]
    if len(data) < 16 or number == 0:
      continue
    kind, _, trainer_class, _, fight, count = struct.unpack_from(
        "<BBHHBB", data, 0)
    team = read_team(teams[number][0], count, kind)
    members = ", ".join(
        "%s %d%s" % (species[s] if s < len(species) else "?%d" % s, level,
                     " (form %d)" % form if form else "")
        for s, level, form in team)
    where = ", ".join(
        "%s (%d)" % (maps.get(zone, "Map"), zone)
        for zone, _ in places.get(number, [])) or "-"
    out.append("%4d | %-34s | %-18s | %-22s | %-8s | %s | %s" % (
        number, constants.get(number, ""),
        classes[trainer_class] if trainer_class < len(classes) else "?",
        (names[number] if number < len(names) else "?") + (
            " (Rematch)" if number in rematches and rematches[number][1] == 1
            else " (Rematch %d)" % rematches[number][1]
            if number in rematches else ""),
        FORMATS[fight] if fight < len(FORMATS) else str(fight), where,
        members))

  text = "\n".join(out) + "\n"
  if len(arguments) > 1:
    with open(arguments[1], "w", encoding="utf-8", newline="\n") as f:
      f.write(text)
    print("%d trainers in %s" % (len(out) - 4, arguments[1]))
  else:
    sys.stdout.reconfigure(encoding="utf-8")
    print(text)
  return 0


if __name__ == "__main__":
  sys.exit(main())
