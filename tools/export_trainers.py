"""Lists all the trainers of the game: number, class, name, format, maps and
team.

The data comes from the archives of the game (TrData, TrPokeData, the texts
and the zone data, which holds the characters of each map). Give the folder
that has them:

  python tools/export_trainers.py <folder of the archives> [output file]
      [--pkpawn <pkpawn-local>] [--sango <source folder>]
      [--enum lib/include/battle/constant/trainer.h] [--maker trainers.txt]

With --pkpawn, the tool also reads the scripts of the maps (it decompiles them
with pkpawn-local) to find the trainers that scripts start: rivals, leaders and
the Elite Four. With --sango, it also reads the rematches of the trainers. With
--enum, it writes the names of all the trainers into the header of TrainerId
(after the marker line): a map, a title and a name. With --maker, it writes the
trainers of the game in the format of trainers.txt (Sango Maker), to change
them in the tool of tools/maker_editor.

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
TEXT_MOVES = 14
TEXT_ITEMS = 114
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


ENUM_MARKER = "  // Generated by tools/export_trainers.py. Do not edit below this line."


def camel(text):
  """Returns a text as a name for C++: 'Pokémon Trainer' -> 'PokemonTrainer'."""
  import unicodedata
  text = unicodedata.normalize("NFKD", text)
  text = "".join(c for c in text if not unicodedata.combining(c))
  words = re.findall(r"[A-Za-z0-9]+", text)
  return "".join(w[0].upper() + w[1:] for w in words)


def words_of(name):
  return " ".join(re.findall(r"[A-Z][a-z]+|[A-Z]+(?![a-z])|\d+|[a-z]+", name))


def write_enum(path, rows, places, maps, rematches):
  """Writes the names of all the trainers after the marker of the header.

  The names above the marker are by hand and stay as they are. The others are
  the map, the title and the name of the trainer: kRoute102YoungsterCalvin.
  Without a map, the name is the number: kTrainer123.
  """
  with open(path, encoding="utf-8") as f:
    text = f.read().replace("\r\n", "\n")
  if ENUM_MARKER not in text:
    raise SystemExit("%s: the marker line is missing" % path)
  head, tail = text.split(ENUM_MARKER)
  rest = tail[tail.index("};"):]
  used = {name for name in re.findall(r"^\s*(k\w+)\s*=", head, re.M)}
  by_hand = {int(n) for n in re.findall(r"^\s*k\w+\s*=\s*(\d+),", head, re.M)}

  groups = {}  # map name -> [(number, name)]
  for number, trainer_class, name in rows:
    if number in by_hand:
      continue
    place = ""
    for zone, _ in places.get(number, []):
      candidate = maps.get(zone, "")
      if candidate and not candidate.startswith("kUnknow"):
        place = candidate[1:]
        break
    if not place:
      groups.setdefault("", []).append((number, "kTrainer%d" % number))
      continue
    title = camel(trainer_class)
    person = camel(name)
    suffix = ""
    if number in rematches:
      step = rematches[number][1]
      suffix = "Rematch" if step == 1 else "Rematch%d" % step
    base = "k" + place + title + person + suffix
    unique = base
    count = 1
    while unique in used:
      count += 1
      unique = "%s%d" % (base, count)
    used.add(unique)
    groups.setdefault(place, []).append((number, unique))

  lines = []
  for place in sorted(groups, key=lambda p: (p == "", groups[p][0][0])):
    lines.append("")
    lines.append("  // " + (words_of(place) if place else "No known map"))
    for number, name in groups[place]:
      lines.append("  %s = %d," % (name, number))
  with open(path, "w", encoding="utf-8", newline="\n") as f:
    f.write(head + ENUM_MARKER + "\n" + "\n".join(lines) + "\n" + rest)
  print("%d names in %s" % (sum(len(v) for v in groups.values()), path))


def write_maker(path, trainers, teams, texts, header):
  """Writes the trainers of the game in the format of trainers.txt.

  A hack maker copies the sections to change into sango/maker/trainers.txt, or
  opens the file in tools/maker_editor. The sections have the names of
  TrainerId. The abilities, natures and forms of the teams are not written.
  """
  species = read_lines(texts[TEXT_SPECIES][0])
  moves = read_lines(texts[TEXT_MOVES][0])
  items = read_lines(texts[TEXT_ITEMS][0])
  names = read_lines(texts[TEXT_TRAINER_NAME][0])
  constants = read_constants(header)
  formats = ["Single", "Double", "Triple", "Rotation", "Horde"]
  out = ["# All the trainers of the game, as they are in the game.",
         "# Copy the sections to change into sango/maker/trainers.txt.", ""]
  for number, parts in enumerate(trainers):
    data = parts[0]
    if len(data) < 22 or number == 0:
      continue
    has_moves = struct.unpack_from("<H", data, 0)[0] & 1
    has_item = struct.unpack_from("<H", data, 0)[0] >> 1 & 1
    fight, count = struct.unpack_from("<BB", data, 6)
    held = struct.unpack_from("<4H", data, 8)
    ai = data[16]
    money = data[21]
    constant = constants.get(number)
    out.append("[%s]" % (words_of(constant[1:]) if constant else number))
    out.append("format = %s" % (formats[fight] if fight < len(formats)
                                 else formats[0]))
    if number < len(names) and names[number]:
      out.append("name = %s" % names[number])
    flags = [n for bit, n in ((1, "Casual"), (2, "Competitive"),
                              (4, "Strategist")) if ai & bit]
    if flags:
      out.append("ai = " + ", ".join(flags))
    used = [items[i] for i in held if 0 < i < len(items)]
    if used:
      out.append("items = " + ", ".join(used))
    out.append("money = %d" % money)
    team = teams[number][0]
    size = 8 + 2 * has_item + 8 * has_moves
    for i in range(count):
      record = team[i * size:(i + 1) * size]
      if len(record) < size:
        break
      level, kind = struct.unpack_from("<HH", record, 2)
      line = "pokemon = %s, level %d" % (
          species[kind] if kind < len(species) else kind, level)
      at = 8
      if has_item:
        item = struct.unpack_from("<H", record, at)[0]
        at += 2
        if 0 < item < len(items):
          line += ", item " + items[item]
      if has_moves:
        ids = struct.unpack_from("<4H", record, at)
        known = [moves[m] for m in ids if 0 < m < len(moves)]
        if known:
          line += ", moves " + " / ".join(known)
      out.append(line)
    out.append("")
  with open(path, "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(out))
  print("%d trainers in %s" % (len(trainers) - 1, path))


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
  maker_path = None
  if "--maker" in arguments:
    at = arguments.index("--maker")
    maker_path = arguments[at + 1]
    del arguments[at:at + 2]
  enum_path = None
  if "--enum" in arguments:
    at = arguments.index("--enum")
    enum_path = arguments[at + 1]
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

  # The first six trainers (the rival Brendan and May) are on Route 103.
  for number in range(1, 7):
    places.setdefault(number, [(25, 0)])

  rematches = {}
  if sango:
    rematches, base_maps = read_rematches(sango)
    for base, zone in base_maps.items():
      places.setdefault(base, [(zone, 0)])
    # A rematch is on the map of the trainer that it replaces.
    for number, (base, _) in rematches.items():
      places[number] = places.get(base, [])

  rows = []  # (number, class, name)
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
    rows.append((number,
                 classes[trainer_class] if trainer_class < len(classes) else "",
                 names[number] if number < len(names) else ""))
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

  if enum_path:
    write_enum(enum_path, rows, places, maps, rematches)
  if maker_path:
    write_maker(maker_path, trainers, teams, texts,
                os.path.join(here, "..", "lib", "include", "battle",
                             "constant", "trainer.h"))

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
