/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file data_files.cc
 * @brief A ROM hack without code: text files that change the game.
 *
 * The declarations are in core/data_files.h.
 */

#include "core/data_files.h"

#include <cstring>

#include "battle/patch/trainer_team.h"
#include "core/data_folder.h"
#include "core/patch/text_editor.h"
#include "core/problems.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/patch/species_editor.h"
#include "system/native/file.h"

namespace core {
namespace {

// The names of the values of the enums: tools/gen_data_names.py.
struct DataName {
  const c8* name;
  u16 value;
};
#include "core/data_names.inc"

struct NameTable {
  const DataName* names;
  u32 count;
};

NameTable GetTable(NameKind kind) {
  switch (kind) {
    case NameKind::kSpecies: return {kSpeciesNames, SIZE(kSpeciesNames)};
    case NameKind::kMove: return {kMoveNames, SIZE(kMoveNames)};
    case NameKind::kItem: return {kItemNames, SIZE(kItemNames)};
    case NameKind::kAbility: return {kAbilityNames, SIZE(kAbilityNames)};
    case NameKind::kNature: return {kNatureNames, SIZE(kNatureNames)};
    case NameKind::kType: return {kTypeNames, SIZE(kTypeNames)};
    case NameKind::kForm: return {kFormNames, SIZE(kFormNames)};
    case NameKind::kTrainer: return {kTrainerNames, SIZE(kTrainerNames)};
    case NameKind::kModel: return {kModelNames, SIZE(kModelNames)};
    case NameKind::kEvolutionMethod:
      return {kEvolutionMethodNames, SIZE(kEvolutionMethodNames)};
    case NameKind::kEncounterMethod:
      return {kEncounterMethodNames, SIZE(kEncounterMethodNames)};
    case NameKind::kFormat: return {kFormatNames, SIZE(kFormatNames)};
    case NameKind::kMap: return {kMapNames, SIZE(kMapNames)};
  }
  return {nullptr, 0};
}

// A part of a line of a file.
struct Span {
  const c8* text;
  u32 length;
};

bool IsSpace(c8 c) { return c == ' ' || c == '\t' || c == '\r'; }

Span Trim(Span span) {
  while (span.length > 0 && IsSpace(span.text[0])) {
    span.text++;
    span.length--;
  }
  while (span.length > 0 && IsSpace(span.text[span.length - 1])) {
    span.length--;
  }
  return span;
}

// Writes the name in lowercase without the spaces and the signs. The letters
// é, è and ê of UTF-8 become e. Returns false when the name is too long.
bool Normalize(Span span, c8* out, u32 capacity) {
  u32 length = 0;
  for (u32 i = 0; i < span.length; i++) {
    c8 c = span.text[i];
    const u8 byte = static_cast<u8>(c);
    if (byte == 0xC3 && i + 1 < span.length) {
      const u8 next = static_cast<u8>(span.text[i + 1]);
      if (next == 0xA9 || next == 0xA8 || next == 0xAA || next == 0x89) {
        c = 'e';
        i++;
      } else {
        continue;
      }
    }
    if (c >= 'A' && c <= 'Z') c = static_cast<c8>(c - 'A' + 'a');
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) continue;
    if (length + 1 >= capacity) return false;
    out[length++] = c;
  }
  out[length] = '\0';
  return true;
}

bool ParseNumber(Span span, u32* value) {
  span = Trim(span);
  if (span.length == 0) return false;
  u32 result = 0;
  for (u32 i = 0; i < span.length; i++) {
    if (span.text[i] < '0' || span.text[i] > '9') return false;
    result = result * 10 + static_cast<u32>(span.text[i] - '0');
  }
  *value = result;
  return true;
}

// Cuts `span` at each `separator`. Returns the number of parts.
u32 Split(Span span, c8 separator, Span* parts, u32 capacity) {
  u32 count = 0;
  u32 start = 0;
  for (u32 i = 0; i <= span.length && count < capacity; i++) {
    if (i < span.length && span.text[i] != separator) continue;
    parts[count++] = Trim(Span{span.text + start, i - start});
    start = i + 1;
  }
  return count;
}

// Cuts "level 8" into "level" and "8".
void SplitWord(Span span, Span* word, Span* rest) {
  span = Trim(span);
  u32 i = 0;
  while (i < span.length && !IsSpace(span.text[i])) i++;
  *word = Span{span.text, i};
  *rest = Trim(Span{span.text + i, span.length - i});
}

bool IsWord(Span span, const c8* word) {
  c8 normalized[32];
  return Normalize(span, normalized, sizeof(normalized)) &&
         std::strcmp(normalized, word) == 0;
}

// The file that the reader reads, for the messages of the problems.
const c8* current_file = "";
u32 current_line = 0;

void Report(const c8* what, Span word) {
  c8 text[24];
  const u32 length = word.length < sizeof(text) - 1 ? word.length
                                                    : sizeof(text) - 1;
  std::memcpy(text, word.text, length);
  text[length] = '\0';
  Problems::Add(u"%s %u: %s %s", current_file, current_line, what, text);
}

bool FindOrReport(NameKind kind, Span span, u32* value, const c8* what) {
  span = Trim(span);
  if (DataNames::Find(kind, span.text, span.length, value)) return true;
  Report(what, span);
  return false;
}

// The text of a file, then its lines one by one.
c8 file_buffer[32 * 1024];

typedef void (*LineHandler)(Span line, bool is_section);

bool ReadLines(const c16* path, const c8* name, LineHandler handler) {
  const u32 size = sys::File::ReadAll(path, file_buffer, sizeof(file_buffer));
  if (size == 0) return false;
  current_file = name;
  if (size >= sizeof(file_buffer)) {
    Problems::Add(u"%s: the file is too big", name);
  }
  u32 start = 0;
  current_line = 0;
  for (u32 i = 0; i <= size; i++) {
    if (i < size && file_buffer[i] != '\n') continue;
    current_line++;
    Span line = {file_buffer + start, i - start};
    start = i + 1;
    // A comment starts with #.
    for (u32 k = 0; k < line.length; k++) {
      if (line.text[k] == '#') line.length = k;
    }
    // A UTF-8 file can start with a mark of 3 bytes.
    if (current_line == 1 && line.length >= 3 &&
        static_cast<u8>(line.text[0]) == 0xEF) {
      line.text += 3;
      line.length -= 3;
    }
    line = Trim(line);
    if (line.length == 0) continue;
    if (line.text[0] == '[' && line.text[line.length - 1] == ']') {
      handler(Span{line.text + 1, line.length - 2}, true);
    } else {
      handler(line, false);
    }
  }
  handler(Span{"", 0}, true);
  return true;
}

// Cuts "key = value". Returns false when the line has no =.
bool SplitKey(Span line, Span* key, Span* value) {
  for (u32 i = 0; i < line.length; i++) {
    if (line.text[i] != '=') continue;
    *key = Trim(Span{line.text, i});
    *value = Trim(Span{line.text + i + 1, line.length - i - 1});
    return true;
  }
  return false;
}

// The texts of the names (UTF-16) that the teams use.
c16 text_pool[4096];
u32 text_pool_size = 0;

const c16* StoreText(Span span) {
  const u32 capacity = SIZE(text_pool) - text_pool_size;
  if (capacity < 2) return nullptr;
  c16* out = text_pool + text_pool_size;
  const u32 count = TextEditor::FromUtf8(span.text, span.length, out, capacity);
  if (count + 1 >= capacity) return nullptr;
  out[count] = 0;
  text_pool_size += count + 1;
  return out;
}

// encounters.txt.

overworld::EncounterSlot slot_pool[2048];
u32 slot_count = 0;

struct EncounterState {
  bool is_valid;
  MapId map_id;
  EncounterMethod method;
  u32 first_slot;
} encounter = {false, MapId::kNone, EncounterMethod::kWalk, 0};

void FlushEncounters() {
  if (encounter.is_valid && slot_count > encounter.first_slot) {
    overworld::WildEncounter::SetTable(encounter.map_id, encounter.method,
                                       slot_pool + encounter.first_slot,
                                       slot_count - encounter.first_slot);
  }
  encounter.is_valid = false;
}

void OnEncounterLine(Span line, bool is_section) {
  if (is_section) {
    FlushEncounters();
    if (line.length == 0) return;
    // [Route 101, Walk]
    Span parts[2];
    if (Split(line, ',', parts, 2) != 2) {
      Report("use [map, method]:", line);
      return;
    }
    u32 map = 0;
    u32 method = 0;
    if (!FindOrReport(NameKind::kMap, parts[0], &map, "unknown map") ||
        !FindOrReport(NameKind::kEncounterMethod, parts[1], &method,
                      "unknown method")) {
      return;
    }
    encounter = {true, static_cast<MapId>(map),
                 static_cast<EncounterMethod>(method), slot_count};
    return;
  }
  if (!encounter.is_valid) return;
  // Pikachu, 3, 5 (, form)
  Span parts[4];
  const u32 count = Split(line, ',', parts, 4);
  u32 species = 0;
  u32 min_level = 0;
  u32 max_level = 0;
  u32 form = 0;
  if (!FindOrReport(NameKind::kSpecies, parts[0], &species,
                    "unknown species")) {
    return;
  }
  if (count < 2 || !ParseNumber(parts[1], &min_level)) {
    Report("give a level after", parts[0]);
    return;
  }
  if (count < 3 || !ParseNumber(parts[2], &max_level)) max_level = min_level;
  if (count >= 4 &&
      !FindOrReport(NameKind::kForm, parts[3], &form, "unknown form")) {
    return;
  }
  if (slot_count >= SIZE(slot_pool)) {
    Report("too many Pokemon:", parts[0]);
    return;
  }
  slot_pool[slot_count++] = overworld::EncounterSlot(
      static_cast<SpeciesId>(species), static_cast<u8>(min_level),
      static_cast<u8>(max_level), static_cast<FormId>(form));
}

// trainers.txt.

battle::TrainerTeam team_pool[256];
u32 team_count = 0;

struct TrainerState {
  battle::TrainerTeam* team;
  TrainerId id;
} trainer = {nullptr, TrainerId::kNone};

void FlushTrainer() {
  if (trainer.team != nullptr) {
    battle::TrainerTeams::Add(trainer.id, trainer.team);
  }
  trainer.team = nullptr;
}

void ParseOpponent(Span value) {
  // Pichu, level 7, item Oran Berry, moves Thunder Shock / Charm, shiny
  Span parts[12];
  const u32 count = Split(value, ',', parts, SIZE(parts));
  u32 species = 0;
  if (!FindOrReport(NameKind::kSpecies, parts[0], &species,
                    "unknown species")) {
    return;
  }
  battle::TrainerOpponent opponent(static_cast<SpeciesId>(species));
  for (u32 i = 1; i < count; i++) {
    Span word;
    Span rest;
    SplitWord(parts[i], &word, &rest);
    u32 number = 0;
    if (IsWord(word, "level") && ParseNumber(rest, &number)) {
      opponent.WithLevel(static_cast<u8>(number));
    } else if (IsWord(word, "item")) {
      if (FindOrReport(NameKind::kItem, rest, &number, "unknown item")) {
        opponent.WithItem(static_cast<ItemId>(number));
      }
    } else if (IsWord(word, "ability")) {
      if (FindOrReport(NameKind::kAbility, rest, &number, "unknown ability")) {
        opponent.WithAbility(static_cast<AbilityId>(number));
      }
    } else if (IsWord(word, "nature")) {
      if (FindOrReport(NameKind::kNature, rest, &number, "unknown nature")) {
        opponent.WithNature(static_cast<Nature>(number));
      }
    } else if (IsWord(word, "form")) {
      if (FindOrReport(NameKind::kForm, rest, &number, "unknown form")) {
        opponent.WithForm(static_cast<FormId>(number));
      }
    } else if (IsWord(word, "ivs") && ParseNumber(rest, &number)) {
      opponent.WithIvs(static_cast<u8>(number));
    } else if (IsWord(word, "shiny")) {
      opponent.WithShiny();
    } else if (IsWord(word, "evs")) {
      Span evs[6];
      u32 values[6] = {};
      if (Split(rest, '/', evs, 6) != 6) {
        Report("give 6 EVs:", rest);
        continue;
      }
      for (u32 k = 0; k < 6; k++) ParseNumber(evs[k], &values[k]);
      opponent.WithEvs(static_cast<u8>(values[0]), static_cast<u8>(values[1]),
                       static_cast<u8>(values[2]), static_cast<u8>(values[3]),
                       static_cast<u8>(values[4]), static_cast<u8>(values[5]));
    } else if (IsWord(word, "moves")) {
      Span names[4];
      MoveId moves[4] = {MoveId::kNone, MoveId::kNone, MoveId::kNone,
                         MoveId::kNone};
      const u32 move_count = Split(rest, '/', names, 4);
      for (u32 k = 0; k < move_count; k++) {
        if (FindOrReport(NameKind::kMove, names[k], &number, "unknown move")) {
          moves[k] = static_cast<MoveId>(number);
        }
      }
      opponent.WithMoves(moves[0], moves[1], moves[2], moves[3]);
    } else {
      Report("unknown value:", parts[i]);
    }
  }
  trainer.team->Add(opponent);
}

void OnTrainerLine(Span line, bool is_section) {
  if (is_section) {
    FlushTrainer();
    if (line.length == 0) return;
    // [Route 102 Kid 1] or [42]
    u32 id = 0;
    if (!FindOrReport(NameKind::kTrainer, line, &id, "unknown trainer")) {
      return;
    }
    if (team_count >= SIZE(team_pool)) {
      Report("too many trainers:", line);
      return;
    }
    trainer.team = &team_pool[team_count++];
    *trainer.team = battle::TrainerTeam(Format::kSingle);
    trainer.id = static_cast<TrainerId>(id);
    return;
  }
  if (trainer.team == nullptr) return;
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  battle::TrainerTeam& team = *trainer.team;
  u32 number = 0;
  if (IsWord(key, "pokemon")) {
    ParseOpponent(value);
  } else if (IsWord(key, "format")) {
    if (FindOrReport(NameKind::kFormat, value, &number, "unknown format")) {
      team.format = static_cast<Format>(number);
    }
  } else if (IsWord(key, "name")) {
    team.WithName(StoreText(value));
  } else if (IsWord(key, "money") && ParseNumber(value, &number)) {
    team.WithMoney(static_cast<u8>(number));
  } else if (IsWord(key, "ai")) {
    Span names[5];
    const u32 count = Split(value, ',', names, 5);
    AiFlags flags = AiFlags::kNone;
    for (u32 k = 0; k < count; k++) {
      if (IsWord(names[k], "casual")) {
        flags |= AiFlags::kCasual;
      } else if (IsWord(names[k], "competitive")) {
        flags |= AiFlags::kCompetitive;
      } else if (IsWord(names[k], "strategist")) {
        flags |= AiFlags::kStrategist;
      } else if (IsWord(names[k], "multi")) {
        flags |= AiFlags::kMulti;
      } else if (IsWord(names[k], "horde")) {
        flags |= AiFlags::kHorde;
      } else {
        Report("unknown AI:", names[k]);
      }
    }
    team.WithAi(flags);
  } else if (IsWord(key, "items")) {
    Span names[4];
    ItemId items[4] = {ItemId::kNone, ItemId::kNone, ItemId::kNone,
                       ItemId::kNone};
    const u32 count = Split(value, ',', names, 4);
    for (u32 k = 0; k < count; k++) {
      if (FindOrReport(NameKind::kItem, names[k], &number, "unknown item")) {
        items[k] = static_cast<ItemId>(number);
      }
    }
    team.WithItems(items[0], items[1], items[2], items[3]);
  } else {
    Report("unknown key:", key);
  }
}

// species.txt.

pokemon::LevelMove move_pool[4096];
u32 move_count = 0;
pokemon::EvolutionStep step_pool[512];
u32 step_count = 0;

struct SpeciesState {
  bool is_valid;
  SpeciesId species;
  FormId form;
} species_state = {false, SpeciesId::kNone, FormId::kNormal};

void ParseMachines(pokemon::Species& species, Span value, bool is_hm,
                   bool can_learn) {
  Span numbers[32];
  const u32 count = Split(value, ',', numbers, SIZE(numbers));
  for (u32 k = 0; k < count; k++) {
    u32 number = 0;
    if (!ParseNumber(numbers[k], &number)) {
      Report("give numbers:", numbers[k]);
      continue;
    }
    if (is_hm) {
      species.WithHm(static_cast<u8>(number), can_learn);
    } else {
      species.WithTm(static_cast<u8>(number), can_learn);
    }
  }
}

void ParseLearnset(pokemon::Species& species, Span value) {
  // 1 Thunder Shock, 5 Quick Attack
  Span parts[pokemon::Species::kMaxLearnsetMoves];
  const u32 count = Split(value, ',', parts, SIZE(parts));
  const u32 first = move_count;
  for (u32 k = 0; k < count; k++) {
    Span level_text;
    Span name;
    SplitWord(parts[k], &level_text, &name);
    u32 level = 0;
    u32 move = 0;
    if (!ParseNumber(level_text, &level)) {
      Report("give a level before the move:", parts[k]);
      continue;
    }
    if (!FindOrReport(NameKind::kMove, name, &move, "unknown move")) continue;
    if (move_count >= SIZE(move_pool)) {
      Report("too many moves:", name);
      return;
    }
    move_pool[move_count++] =
        pokemon::LevelMove(static_cast<u8>(level), static_cast<MoveId>(move));
  }
  species.WithLearnset(move_pool + first, move_count - first);
}

void ParseEvolutions(pokemon::Species& species, Span value) {
  // Level Up / 30 / Raichu, Item / Thunder Stone / Raichu
  Span evolutions[pokemon::Species::kMaxEvolutions];
  const u32 count = Split(value, ',', evolutions, SIZE(evolutions));
  const u32 first = step_count;
  for (u32 k = 0; k < count; k++) {
    Span parts[3];
    if (Split(evolutions[k], '/', parts, 3) != 3) {
      Report("use method / value / species:", evolutions[k]);
      continue;
    }
    u32 method = 0;
    u32 into = 0;
    u32 number = 0;
    if (!FindOrReport(NameKind::kEvolutionMethod, parts[0], &method,
                      "unknown method") ||
        !FindOrReport(NameKind::kSpecies, parts[2], &into,
                      "unknown species")) {
      continue;
    }
    // The value: a number, an item, a move or a species.
    const Span v = parts[1];
    if (!ParseNumber(v, &number) &&
        !DataNames::Find(NameKind::kItem, v.text, v.length, &number) &&
        !DataNames::Find(NameKind::kMove, v.text, v.length, &number) &&
        !DataNames::Find(NameKind::kSpecies, v.text, v.length, &number)) {
      Report("unknown value:", v);
      continue;
    }
    if (step_count >= SIZE(step_pool)) {
      Report("too many evolutions:", evolutions[k]);
      return;
    }
    step_pool[step_count++] = pokemon::EvolutionStep(
        static_cast<EvolutionMethod>(method), static_cast<u16>(number),
        static_cast<SpeciesId>(into));
  }
  species.WithEvolutions(step_pool + first, step_count - first);
}

void OnSpeciesLine(Span line, bool is_section) {
  if (is_section) {
    species_state.is_valid = false;
    if (line.length == 0) return;
    // [Pikachu] or [Charizard, Charizard Megax]
    Span parts[2];
    const u32 count = Split(line, ',', parts, 2);
    u32 species = 0;
    u32 form = 0;
    if (!FindOrReport(NameKind::kSpecies, parts[0], &species,
                      "unknown species")) {
      return;
    }
    if (count == 2 &&
        !FindOrReport(NameKind::kForm, parts[1], &form, "unknown form")) {
      return;
    }
    species_state = {true, static_cast<SpeciesId>(species),
                     static_cast<FormId>(form)};
    return;
  }
  if (!species_state.is_valid) return;
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  pokemon::Species species(species_state.species, species_state.form);
  u32 number = 0;
  if (IsWord(key, "stats")) {
    Span parts[6];
    u32 stats[6] = {};
    if (Split(value, ',', parts, 6) != 6) {
      Report("give 6 stats:", value);
      return;
    }
    for (u32 k = 0; k < 6; k++) ParseNumber(parts[k], &stats[k]);
    species.WithStats(static_cast<u8>(stats[0]), static_cast<u8>(stats[1]),
                      static_cast<u8>(stats[2]), static_cast<u8>(stats[3]),
                      static_cast<u8>(stats[4]), static_cast<u8>(stats[5]));
  } else if (IsWord(key, "types")) {
    Span parts[2];
    const u32 count = Split(value, ',', parts, 2);
    u32 first = 0;
    u32 second = 0;
    if (!FindOrReport(NameKind::kType, parts[0], &first, "unknown type")) {
      return;
    }
    second = first;
    if (count == 2 &&
        !FindOrReport(NameKind::kType, parts[1], &second, "unknown type")) {
      return;
    }
    species.WithTypes(static_cast<TypeId>(first), static_cast<TypeId>(second));
  } else if (IsWord(key, "abilities")) {
    Span parts[3];
    u32 abilities[3] = {};
    const u32 count = Split(value, ',', parts, 3);
    for (u32 k = 0; k < 3; k++) {
      // One or two abilities: the last one also fills the next places.
      const Span name = parts[k < count ? k : count - 1];
      if (!FindOrReport(NameKind::kAbility, name, &abilities[k],
                        "unknown ability")) {
        return;
      }
    }
    species.WithAbilities(static_cast<AbilityId>(abilities[0]),
                          static_cast<AbilityId>(abilities[1]),
                          static_cast<AbilityId>(abilities[2]));
  } else if (IsWord(key, "catchrate") && ParseNumber(value, &number)) {
    species.WithCatchRate(static_cast<u8>(number));
  } else if (IsWord(key, "tms")) {
    ParseMachines(species, value, false, true);
  } else if (IsWord(key, "tmsoff")) {
    ParseMachines(species, value, false, false);
  } else if (IsWord(key, "hms")) {
    ParseMachines(species, value, true, true);
  } else if (IsWord(key, "hmsoff")) {
    ParseMachines(species, value, true, false);
  } else if (IsWord(key, "learnset")) {
    ParseLearnset(species, value);
  } else if (IsWord(key, "evolutions")) {
    ParseEvolutions(species, value);
  } else {
    Report("unknown key:", key);
  }
}

} // namespace

bool DataNames::Find(NameKind kind, const c8* text, u32 length, u32* value) {
  const Span span = Trim(Span{text, length});
  if (ParseNumber(span, value)) return true;
  c8 name[48];
  if (!Normalize(span, name, sizeof(name)) || name[0] == '\0') return false;
  const NameTable table = GetTable(kind);
  u32 low = 0;
  u32 high = table.count;
  while (low < high) {
    const u32 middle = (low + high) / 2;
    const s32 order = std::strcmp(table.names[middle].name, name);
    if (order == 0) {
      *value = table.names[middle].value;
      return true;
    }
    if (order < 0) {
      low = middle + 1;
    } else {
      high = middle;
    }
  }
  return false;
}

bool DataFiles::LoadEncounters(const c16* path) {
  return ReadLines(path, "encounters.txt", OnEncounterLine);
}

bool DataFiles::LoadTrainers(const c16* path) {
  return ReadLines(path, "trainers.txt", OnTrainerLine);
}

bool DataFiles::LoadSpecies(const c16* path) {
  return ReadLines(path, "species.txt", OnSpeciesLine);
}

void DataFiles::Load() {
  LoadEncounters(SANGO_DATA_PATH(u"encounters.txt"));
  LoadTrainers(SANGO_DATA_PATH(u"trainers.txt"));
  LoadSpecies(SANGO_DATA_PATH(u"species.txt"));
}

} // namespace core
