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
 * @file maker_reader.cc
 * @brief Reads the text files of Sango Maker.
 *
 * The declarations are in core/maker_reader.h. The file has no game
 * structure: the tests of tests/ compile it on a computer.
 */

#include "core/maker_reader.h"

#include <cstring>

#include "core/utf8.h"
#ifdef __3DS__
#include <cstdlib>
#include "system/native/file.h"
#endif

namespace core {
namespace maker {
namespace {

// The names of the values of the enums: tools/gen_data_names.py.
struct DataName {
  const c8* name;
  u16 value;
};

struct NameTable {
  const DataName* names;
  u32 count;
};

// The plugin for the console reads the tables from the files of
// sdmc:/sango/names, when it reads the data files (the strings are not in the
// plugin). The tests of the computer and the plugin for X and Y use the tables
// of data_names.inc.
#if defined(__3DS__) && !defined(GAME_XY)
#define SANGO_NAME_FILES 1
#endif

#ifndef SANGO_NAME_FILES
#include "core/data_names.inc"
#else
constexpr u32 kNameKindCount = 16;
const c8* const kTableFiles[kNameKindCount] = {
    "species", "move", "item", "ability", "nature", "type", "form",
    "trainer", "model", "evolution", "method", "format", "map", "music",
    "trainermodel", "trainerclass"};

struct LoadedTable {
  u8* data;
  NameTable table;
  bool is_tried;
};
LoadedTable loaded_tables[kNameKindCount];

// Reads the file of a kind of names: its size, the number of names, the
// entries (an offset, a value and a padding) and the names. The offsets become
// addresses, so the entries are an array of DataName.
NameTable LoadTable(NameKind kind) {
  const u32 index = static_cast<u32>(kind);
  if (index >= kNameKindCount) return {nullptr, 0};
  LoadedTable& loaded = loaded_tables[index];
  if (loaded.is_tried) return loaded.table;
  loaded.is_tried = true;
  loaded.table = {nullptr, 0};

  c16 path[64];
  static const c16 kPrefix[] = u"sdmc:/sango/names/";
  u32 length = 0;
  for (const c16* p = kPrefix; *p != 0; p++) path[length++] = *p;
  for (const c8* p = kTableFiles[index]; *p != 0; p++) {
    path[length++] = static_cast<c16>(static_cast<u8>(*p));
  }
  static const c16 kSuffix[] = u".tab";
  for (const c16* p = kSuffix; *p != 0; p++) path[length++] = *p;
  path[length] = 0;

  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) return loaded.table;
  u32 header[2] = {};
  if (file.Read(header, 8) != 8) return loaded.table;
  const u32 size = header[0];
  const u32 count = header[1];
  if (size < 8 + 8 * count || size > 96 * 1024 || count > 20000) {
    return loaded.table;
  }
  u8* data = static_cast<u8*>(std::malloc(size));
  if (data == nullptr) return loaded.table;
  std::memcpy(data, header, 8);
  if (size > 8 && file.Read(data + 8, size - 8) !=
                      static_cast<s32>(size - 8)) {
    std::free(data);
    return loaded.table;
  }
  u32* entries = reinterpret_cast<u32*>(data + 8);
  for (u32 i = 0; i < count; i++) {
    if (entries[2 * i] >= size) {
      std::free(data);
      return loaded.table;
    }
    entries[2 * i] += reinterpret_cast<u32>(data);
  }
  data[size - 1] = 0;
  loaded.data = data;
  loaded.table = {reinterpret_cast<const DataName*>(entries), count};
  return loaded.table;
}

void ReleaseTables() {
  for (u32 i = 0; i < kNameKindCount; i++) {
    std::free(loaded_tables[i].data);
    loaded_tables[i] = {nullptr, {nullptr, 0}, false};
  }
}
#endif

NameTable GetTable(NameKind kind) {
#ifdef SANGO_NAME_FILES
  return LoadTable(kind);
#else
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
    case NameKind::kMusic: return {kMusicNames, SIZE(kMusicNames)};
    case NameKind::kTrainerModel:
      return {kXyTrainerModelNames, SIZE(kXyTrainerModelNames)};
    case NameKind::kTrainerClass:
      return {kTrainerClassNames, SIZE(kTrainerClassNames)};
  }
  return {nullptr, 0};
#endif
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

// The sink of the file that the reader reads, the name of the file and the
// line, for the messages of the problems.
Sink* sink = nullptr;
const c8* current_file = "";
u32 current_line = 0;

void Report(const c8* what, Span word) {
  c8 text[24];
  const u32 length = word.length < sizeof(text) - 1 ? word.length
                                                    : sizeof(text) - 1;
  std::memcpy(text, word.text, length);
  text[length] = '\0';
  sink->OnProblem(current_file, current_line, what, text);
}

bool FindOrReport(NameKind kind, Span span, u32* value, const c8* what) {
  span = Trim(span);
  if (DataNames::Find(kind, span.text, span.length, value)) return true;
  Report(what, span);
  return false;
}

// The lines of a text, one by one: the comments go, and the sections
// ("[...]") have `is_section` true. The handler gets an empty section at the end.
typedef void (*LineHandler)(Span line, bool is_section);

// True when the file that ReadLines() reads has had a [section] line.
bool is_section_seen = false;

// A line of a file with sections, before the first section: nothing uses it.
// Tell the user, who may have forgotten the [section] line.
void ReportOrphan(Span line) {
  if (is_section_seen) return;
  Report("write a [section] line before this line:", line);
}

void ReadLines(const c8* text, u32 size, const c8* name, Sink& file_sink,
               LineHandler handler) {
  sink = &file_sink;
  current_file = name;
  u32 start = 0;
  current_line = 0;
  is_section_seen = false;
  for (u32 i = 0; i <= size; i++) {
    if (i < size && text[i] != '\n') continue;
    current_line++;
    Span line = {text + start, i - start};
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
      is_section_seen = true;
      handler(Span{line.text + 1, line.length - 2}, true);
    } else {
      handler(line, false);
    }
  }
  handler(Span{"", 0}, true);
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
  const u32 count = Utf8ToUtf16(span.text, span.length, out, capacity);
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
    sink->OnEncounterTable(encounter.map_id, encounter.method,
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
  if (!encounter.is_valid) {
    ReportOrphan(line);
    return;
  }
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

// The number of trainers that the files can change.
constexpr u32 kMaxTrainers = 256;
u32 team_count = 0;
bool is_trainer_open = false;

void FlushTrainer() {
  if (is_trainer_open) sink->OnTrainerEnd();
  is_trainer_open = false;
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
  Opponent opponent = {};
  opponent.species = static_cast<SpeciesId>(species);
  for (u32 i = 1; i < count; i++) {
    Span word;
    Span rest;
    SplitWord(parts[i], &word, &rest);
    u32 number = 0;
    if (IsWord(word, "level") && ParseNumber(rest, &number)) {
      opponent.has_level = true;
      opponent.level = static_cast<u8>(number);
    } else if (IsWord(word, "item")) {
      if (FindOrReport(NameKind::kItem, rest, &number, "unknown item")) {
        opponent.has_item = true;
        opponent.item = static_cast<ItemId>(number);
      }
    } else if (IsWord(word, "ability")) {
      if (FindOrReport(NameKind::kAbility, rest, &number, "unknown ability")) {
        opponent.has_ability = true;
        opponent.ability = static_cast<AbilityId>(number);
      }
    } else if (IsWord(word, "nature")) {
      if (FindOrReport(NameKind::kNature, rest, &number, "unknown nature")) {
        opponent.has_nature = true;
        opponent.nature = static_cast<Nature>(number);
      }
    } else if (IsWord(word, "form")) {
      if (FindOrReport(NameKind::kForm, rest, &number, "unknown form")) {
        opponent.has_form = true;
        opponent.form = static_cast<FormId>(number);
      }
    } else if (IsWord(word, "ivs") && ParseNumber(rest, &number)) {
      opponent.has_ivs = true;
      opponent.ivs = static_cast<u8>(number);
    } else if (IsWord(word, "shiny")) {
      opponent.is_shiny = true;
    } else if (IsWord(word, "evs")) {
      Span evs[6];
      u32 values[6] = {};
      if (Split(rest, '/', evs, 6) != 6) {
        Report("give 6 EVs:", rest);
        continue;
      }
      for (u32 k = 0; k < 6; k++) ParseNumber(evs[k], &values[k]);
      opponent.has_evs = true;
      for (u32 k = 0; k < 6; k++) opponent.evs[k] = static_cast<u8>(values[k]);
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
      opponent.has_moves = true;
      for (u32 k = 0; k < 4; k++) opponent.moves[k] = moves[k];
    } else {
      Report("unknown value:", parts[i]);
    }
  }
  sink->OnTrainerPokemon(opponent);
}

// The names of the new trainers ([new AZ]), in the normalized form. DataNames
// finds them for the other files (characters.txt, story.txt).
struct NewTrainerName {
  c8 name[48];
  u16 id;
};
NewTrainerName new_trainer_names[kMaxNewTrainers];
u32 new_trainer_count = 0;

bool FindNewTrainer(const c8* normalized, u32* value) {
  for (u32 i = 0; i < new_trainer_count; i++) {
    if (std::strcmp(new_trainer_names[i].name, normalized) == 0) {
      *value = new_trainer_names[i].id;
      return true;
    }
  }
  return false;
}

// The names of the new classes ([new class Warden, Lass]).
struct NewClassName {
  c8 name[48];
  u16 id;
};
NewClassName new_class_names[kMaxNewClasses];
u32 new_class_count = 0;
bool is_class_open = false;
TrainerClassId open_class = TrainerClassId::kCount;

bool FindNewClass(const c8* normalized, u32* value) {
  for (u32 i = 0; i < new_class_count; i++) {
    if (std::strcmp(new_class_names[i].name, normalized) == 0) {
      *value = new_class_names[i].id;
      return true;
    }
  }
  return false;
}

// [new class Warden, Lass]: a trainer class that the game does not have, with
// the data of another class.
void OpenNewClass(Span text) {
  Span parts[3];
  const u32 count = Split(text, ',', parts, 3);
  if (count > 2) {
    Report("use [new class name, class]:", text);
    return;
  }
  const Span name = parts[0];
  c8 normalized[sizeof(new_class_names[0].name)];
  if (!Normalize(name, normalized, sizeof(normalized)) ||
      normalized[0] == '\0') {
    Report("give a name:", name);
    return;
  }
  u32 used = 0;
  if (DataNames::Find(NameKind::kTrainerClass, name.text, name.length, &used)) {
    Report("the name is used:", name);
    return;
  }
  c16 text16[24];
  if (Utf8ToUtf16(name.text, name.length, text16, SIZE(text16)) > 20) {
    Report("use a name of 20 letters at most:", name);
    return;
  }
  u32 from = static_cast<u32>(TrainerClassId::kYoungster);
  if (count == 2 && !FindOrReport(NameKind::kTrainerClass, parts[1], &from,
                                  "unknown trainer class")) {
    return;
  }
  if (new_class_count >= SIZE(new_class_names)) {
    Report("too many new classes:", name);
    return;
  }
  const TrainerClassId id =
      sink->OnNewClass(text16, static_cast<TrainerClassId>(from));
  if (id == TrainerClassId::kCount) {
    Report("no free class for:", name);
    return;
  }
  NewClassName& entry = new_class_names[new_class_count++];
  std::memcpy(entry.name, normalized, sizeof(entry.name));
  entry.id = static_cast<u16>(id);
  is_class_open = true;
  open_class = id;
}

// A line of a class: title, overworld, picture, introduction, music.
void ParseClassLine(Span line) {
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  u32 number = 0;
  if (IsWord(key, "title")) {
    sink->OnClassTitle(open_class, StoreText(value));
  } else if (IsWord(key, "overworld")) {
    if (FindOrReport(NameKind::kModel, value, &number, "unknown model")) {
      sink->OnClassOverworld(open_class, static_cast<ModelId>(number));
    }
  } else if (IsWord(key, "picture")) {
    if (FindOrReport(NameKind::kTrainerClass, value, &number,
                     "unknown trainer class")) {
      sink->OnClassPicture(open_class, static_cast<TrainerClassId>(number));
    }
  } else if (IsWord(key, "introduction")) {
    if (FindOrReport(NameKind::kTrainerClass, value, &number,
                     "unknown trainer class")) {
      sink->OnClassIntroduction(open_class,
                                static_cast<TrainerClassId>(number));
    }
  } else if (IsWord(key, "music")) {
    if (FindOrReport(NameKind::kTrainerClass, value, &number,
                     "unknown trainer class")) {
      sink->OnClassMusic(open_class, static_cast<TrainerClassId>(number));
    }
  } else {
    Report("unknown key:", key);
  }
}

// [new AZ]: a trainer that the game does not have. The name of the trainer is
// the name of the section.
void OpenNewTrainer(Span name) {
  c8 normalized[sizeof(new_trainer_names[0].name)];
  if (!Normalize(name, normalized, sizeof(normalized)) ||
      normalized[0] == '\0') {
    Report("give a name:", name);
    return;
  }
  u32 used = 0;
  if (DataNames::Find(NameKind::kTrainer, name.text, name.length, &used)) {
    Report("the name is used:", name);
    return;
  }
  c16 text16[20];
  if (Utf8ToUtf16(name.text, name.length, text16, SIZE(text16)) >
      SIZE(text16) - 2) {
    Report("use a name of 16 letters at most:", name);
    return;
  }
  if (new_trainer_count >= SIZE(new_trainer_names)) {
    Report("too many new trainers:", name);
    return;
  }
  const TrainerId id = sink->OnNewTrainer(text16);
  if (id == TrainerId::kNone) {
    Report("no free trainer for:", name);
    return;
  }
  NewTrainerName& entry = new_trainer_names[new_trainer_count++];
  std::memcpy(entry.name, normalized, sizeof(entry.name));
  entry.id = static_cast<u16>(id);
  team_count++;
  is_trainer_open = true;
  sink->OnTrainer(id);
  sink->OnTrainerName(StoreText(name));
}

void OnTrainerLine(Span line, bool is_section) {
  if (is_section) {
    FlushTrainer();
    if (line.length == 0) return;
    Span first;
    Span rest;
    SplitWord(line, &first, &rest);
    if (IsWord(first, "new") && rest.length != 0) {
      Span second;
      Span rest2;
      SplitWord(rest, &second, &rest2);
      if (IsWord(second, "class") && rest2.length != 0) {
        OpenNewClass(rest2);
      } else {
        OpenNewTrainer(rest);
      }
      return;
    }
    // [Route 102 Kid 1] or [42]
    u32 id = 0;
    if (!FindOrReport(NameKind::kTrainer, line, &id, "unknown trainer")) {
      return;
    }
    if (team_count >= kMaxTrainers) {
      Report("too many trainers:", line);
      return;
    }
    team_count++;
    is_trainer_open = true;
    sink->OnTrainer(static_cast<TrainerId>(id));
    return;
  }
  if (is_class_open) {
    ParseClassLine(line);
    return;
  }
  if (!is_trainer_open) {
    ReportOrphan(line);
    return;
  }
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  u32 number = 0;
  if (IsWord(key, "pokemon")) {
    ParseOpponent(value);
  } else if (IsWord(key, "format")) {
    if (FindOrReport(NameKind::kFormat, value, &number, "unknown format")) {
      sink->OnTrainerFormat(static_cast<Format>(number));
    }
  } else if (IsWord(key, "name")) {
    sink->OnTrainerName(StoreText(value));
  } else if (IsWord(key, "money") && ParseNumber(value, &number)) {
    sink->OnTrainerMoney(static_cast<u8>(number));
  } else if (IsWord(key, "model")) {
    if (FindOrReport(NameKind::kTrainerModel, value, &number,
                     "unknown trainer model")) {
      sink->OnTrainerModel(static_cast<XyTrainerModelId>(number));
    }
  } else if (IsWord(key, "class")) {
    if (FindOrReport(NameKind::kTrainerClass, value, &number,
                     "unknown trainer class")) {
      sink->OnTrainerClass(static_cast<TrainerClassId>(number));
    }
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
    sink->OnTrainerAi(flags);
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
    sink->OnTrainerItems(items, 4);
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

// The names of the new species ([new Sangoat, Skiddo]), in the normalized
// form. DataNames finds them for the other files.
struct NewSpeciesName {
  c8 name[24];
  u16 id;
};
NewSpeciesName new_species_names[kMaxNewSpecies];
u32 new_species_count = 0;

bool FindNewSpecies(const c8* normalized, u32* value) {
  for (u32 i = 0; i < new_species_count; i++) {
    if (std::strcmp(new_species_names[i].name, normalized) == 0) {
      *value = new_species_names[i].id;
      return true;
    }
  }
  return false;
}

// Finds a name of the game (or a number), without the new species.
bool FindGameName(NameKind kind, Span span, u32* value);

// [new Sangoat, Skiddo] or [new Sangoat, Skiddo, Gogoat]: the name, the
// species of the first data, and the species of the model.
void OpenNewSpecies(Span text) {
  Span parts[4];
  const u32 count = Split(text, ',', parts, SIZE(parts));
  if (count < 2 || count > 3) {
    Report("use [new name, species, model]:", text);
    return;
  }
  const Span name = parts[0];
  c8 normalized[sizeof(new_species_names[0].name)];
  if (!Normalize(name, normalized, sizeof(normalized)) ||
      normalized[0] == '\0') {
    Report("give a name:", name);
    return;
  }
  u32 used = 0;
  if (DataNames::Find(NameKind::kSpecies, name.text, name.length, &used)) {
    Report("the name is used:", name);
    return;
  }
  c16 text16[kMaxSpeciesNameLength + 2];
  if (Utf8ToUtf16(name.text, name.length, text16, SIZE(text16)) >
      kMaxSpeciesNameLength) {
    Report("use a name of 12 letters at most:", name);
    return;
  }
  u32 data_from = 0;
  u32 model = 0;
  if (!FindGameName(NameKind::kSpecies, parts[1], &data_from)) {
    Report("unknown species", parts[1]);
    return;
  }
  model = data_from;
  if (count == 3 && !FindGameName(NameKind::kSpecies, parts[2], &model)) {
    Report("unknown species", parts[2]);
    return;
  }
  if (new_species_count >= SIZE(new_species_names)) {
    Report("too many new species:", name);
    return;
  }
  const SpeciesId id = sink->OnNewSpecies(
      text16, static_cast<SpeciesId>(data_from), static_cast<SpeciesId>(model));
  if (id == SpeciesId::kNone) return;
  NewSpeciesName& entry = new_species_names[new_species_count++];
  std::memcpy(entry.name, normalized, sizeof(entry.name));
  entry.id = static_cast<u16>(id);
  species_state = {true, id, FormId::kNormal};
}

void ParseMachines(Span value, bool is_hm, bool can_learn) {
  Span numbers[32];
  const u32 count = Split(value, ',', numbers, SIZE(numbers));
  for (u32 k = 0; k < count; k++) {
    u32 number = 0;
    if (!ParseNumber(numbers[k], &number)) {
      Report("give numbers:", numbers[k]);
      continue;
    }
    sink->OnSpeciesMachine(species_state.species, species_state.form,
                           static_cast<u8>(number), is_hm, can_learn);
  }
}

// The number of moves of a learnset and of evolutions of a species (see
// pokemon::Species).
constexpr u32 kMaxLearnsetMoves = 24;
constexpr u32 kMaxEvolutions = 8;

void ParseLearnset(Span value) {
  // 1 Thunder Shock, 5 Quick Attack
  Span parts[kMaxLearnsetMoves];
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
  sink->OnSpeciesLearnset(species_state.species, species_state.form,
                          move_pool + first, move_count - first);
}

void ParseEvolutions(Span value) {
  // Level Up / 30 / Raichu, Item / Thunder Stone / Raichu
  Span evolutions[kMaxEvolutions];
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
  sink->OnSpeciesEvolutions(species_state.species, step_pool + first,
                            step_count - first);
}

void OnSpeciesLine(Span line, bool is_section) {
  if (is_section) {
    species_state.is_valid = false;
    if (line.length == 0) return;
    Span word;
    Span rest;
    SplitWord(line, &word, &rest);
    if (IsWord(word, "new") && rest.length != 0) {
      OpenNewSpecies(rest);
      return;
    }
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
  if (!species_state.is_valid) {
    ReportOrphan(line);
    return;
  }
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  const SpeciesId species = species_state.species;
  const FormId form = species_state.form;
  u32 number = 0;
  if (IsWord(key, "stats")) {
    Span parts[6];
    u32 stats[6] = {};
    if (Split(value, ',', parts, 6) != 6) {
      Report("give 6 stats:", value);
      return;
    }
    for (u32 k = 0; k < 6; k++) ParseNumber(parts[k], &stats[k]);
    u8 values[6];
    for (u32 k = 0; k < 6; k++) values[k] = static_cast<u8>(stats[k]);
    sink->OnSpeciesStats(species, form, values);
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
    sink->OnSpeciesTypes(species, form, static_cast<TypeId>(first),
                         static_cast<TypeId>(second));
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
    sink->OnSpeciesAbilities(species, form,
                             static_cast<AbilityId>(abilities[0]),
                             static_cast<AbilityId>(abilities[1]),
                             static_cast<AbilityId>(abilities[2]));
  } else if (IsWord(key, "catchrate") && ParseNumber(value, &number)) {
    sink->OnSpeciesCatchRate(species, form, static_cast<u8>(number));
  } else if (IsWord(key, "tms")) {
    ParseMachines(value, false, true);
  } else if (IsWord(key, "tmsoff")) {
    ParseMachines(value, false, false);
  } else if (IsWord(key, "hms")) {
    ParseMachines(value, true, true);
  } else if (IsWord(key, "hmsoff")) {
    ParseMachines(value, true, false);
  } else if (IsWord(key, "learnset")) {
    ParseLearnset(value);
  } else if (IsWord(key, "evolutions")) {
    ParseEvolutions(value);
  } else {
    Report("unknown key:", key);
  }
}

// story.txt.

Instruction instruction_pool[2048];
u32 instruction_count = 0;
Story stories[128];
u32 story_count = 0;
c16 story_text_pool[8192];
u32 story_text_size = 0;

// The "if" and "else" that wait for their "end".
u16 open_blocks[16];
u32 open_block_count = 0;
bool is_story_open = false;

// Copies a text of a script. "\n" is a new line.
const c16* StoreStoryText(Span span) {
  const u32 capacity = SIZE(story_text_pool) - story_text_size;
  if (capacity < 2) return nullptr;
  c16* out = story_text_pool + story_text_size;
  const u32 count = Utf8ToUtf16(span.text, span.length, out, capacity);
  if (count + 1 >= capacity) return nullptr;
  u32 write = 0;
  for (u32 read = 0; read < count; read++) {
    if (out[read] == u'\\' && read + 1 < count && out[read + 1] == u'n') {
      out[write++] = u'\n';
      read++;
    } else {
      out[write++] = out[read];
    }
  }
  out[write] = 0;
  story_text_size += write + 1;
  return out;
}

// Cuts "Rare Candy 5" into "Rare Candy" and 5. Without a number at the end,
// `number` keeps its value.
Span SplitLastNumber(Span span, u32* number) {
  span = Trim(span);
  u32 i = span.length;
  while (i > 0 && span.text[i - 1] >= '0' && span.text[i - 1] <= '9') i--;
  if (i == span.length || i == 0 || !IsSpace(span.text[i - 1])) return span;
  ParseNumber(Span{span.text + i, span.length - i}, number);
  return Trim(Span{span.text, i});
}

bool ParseFacing(Span span, u8* facing) {
  static const c8* const kNames[] = {"up", "down", "left", "right"};
  for (u32 i = 0; i < SIZE(kNames); i++) {
    if (IsWord(span, kNames[i])) {
      *facing = static_cast<u8>(i);
      return true;
    }
  }
  return false;
}

void CloseStory() {
  if (!is_story_open) return;
  if (open_block_count != 0) {
    Report("an if has no end in script", Span{"", 0});
  }
  Story& story = stories[story_count - 1];
  story.count = static_cast<u16>(instruction_count - story.first);
  is_story_open = false;
}

bool ParseCondition(Span span, Instruction* instruction) {
  Span word;
  Span rest;
  SplitWord(span, &word, &rest);
  u32 number = 0;
  if (IsWord(word, "yes")) {
    instruction->condition = Condition::kYes;
  } else if (IsWord(word, "no")) {
    instruction->condition = Condition::kNo;
  } else if (IsWord(word, "won")) {
    instruction->condition = Condition::kWon;
  } else if (IsWord(word, "lost")) {
    instruction->condition = Condition::kLost;
  } else if (IsWord(word, "choice") && ParseNumber(rest, &number)) {
    instruction->condition = Condition::kChoice;
    instruction->a = static_cast<u16>(number);
  } else if (IsWord(word, "flag") && ParseNumber(rest, &number)) {
    instruction->condition = Condition::kFlag;
    instruction->a = static_cast<u16>(number);
  } else if (IsWord(word, "not")) {
    Span flag_word;
    Span flag_number;
    SplitWord(rest, &flag_word, &flag_number);
    if (!IsWord(flag_word, "flag") || !ParseNumber(flag_number, &number)) {
      return false;
    }
    instruction->condition = Condition::kNotFlag;
    instruction->a = static_cast<u16>(number);
  } else if (IsWord(word, "money") && ParseNumber(rest, &number)) {
    instruction->condition = Condition::kMoney;
    instruction->a = static_cast<u16>(number & 0xFFFF);
    instruction->b = static_cast<u16>(number >> 16);
  } else if (IsWord(word, "has")) {
    u32 count = 1;
    const Span name = SplitLastNumber(rest, &count);
    if (!FindOrReport(NameKind::kItem, name, &number, "unknown item")) {
      return false;
    }
    instruction->condition = Condition::kHasItem;
    instruction->a = static_cast<u16>(number);
    instruction->b = static_cast<u16>(count);
  } else if (IsWord(word, "var")) {
    // var 3 >= 10
    Span parts[3];
    u32 count = 0;
    Span remaining = rest;
    while (count < 3 && remaining.length > 0) {
      SplitWord(remaining, &parts[count], &remaining);
      count++;
    }
    u32 variable = 0;
    u32 value = 0;
    if (count != 3 || !ParseNumber(parts[0], &variable) ||
        !ParseNumber(parts[2], &value)) {
      return false;
    }
    struct Operator {
      const c8* text;
      Condition condition;
    };
    static const Operator kOperators[] = {
        {"=", Condition::kVarEqual},    {"==", Condition::kVarEqual},
        {"!=", Condition::kVarNotEqual}, {">=", Condition::kVarAtLeast},
        {"<=", Condition::kVarAtMost},  {">", Condition::kVarMore},
        {"<", Condition::kVarLess},
    };
    instruction->condition = Condition::kNone;
    for (const Operator& op : kOperators) {
      if (parts[1].length == std::strlen(op.text) &&
          std::memcmp(parts[1].text, op.text, parts[1].length) == 0) {
        instruction->condition = op.condition;
      }
    }
    if (instruction->condition == Condition::kNone) return false;
    instruction->a = static_cast<u16>(variable);
    instruction->b = static_cast<u16>(value);
  } else {
    return false;
  }
  return true;
}

// The words of a command line: "give item Rare Candy 3" gives the first word
// "give", the second word "item", `rest` "item Rare Candy 3" and `tail`
// "Rare Candy 3".
struct CommandWords {
  Span rest; ///< The text after the first word.
  Span tail; ///< The text after the second word.
  /// For the commands with two first words ("give/take"): true for the first
  /// one (give), false for the second one (take).
  bool is_first_choice;
};

// The commands. Each function reads the values of one command into `in`, and
// returns false when they are not correct.
bool ParseText(const CommandWords& w, Instruction* in) {
  in->text = StoreStoryText(w.rest);
  return in->text != nullptr;
}

bool ParseSay(const CommandWords& w, Instruction* in) {
  in->op = Op::kSay;
  return ParseText(w, in);
}

bool ParseSign(const CommandWords& w, Instruction* in) {
  in->op = Op::kSign;
  return ParseText(w, in);
}

bool ParseAsk(const CommandWords& w, Instruction* in) {
  in->op = Op::kAsk;
  return ParseText(w, in);
}

bool ParseChoose(const CommandWords& w, Instruction* in) {
  // choose Bulbasaur / Charmander / Squirtle: the texts follow each other.
  Span choices[8];
  const u32 count = Split(w.rest, '/', choices, SIZE(choices));
  in->op = Op::kChoose;
  in->a = static_cast<u16>(count);
  for (u32 k = 0; k < count; k++) {
    const c16* text = StoreStoryText(choices[k]);
    if (text == nullptr) return false;
    if (k == 0) in->text = text;
  }
  return true;
}

bool ParseIf(const CommandWords& w, Instruction* in) {
  in->op = Op::kIf;
  return ParseCondition(w.rest, in);
}

// The commands without values.
template <Op kOp>
bool ParseSimple(const CommandWords&, Instruction* in) {
  in->op = kOp;
  return true;
}

bool ParseFade(const CommandWords& w, Instruction* in) {
  Span second;
  Span tail;
  SplitWord(w.rest, &second, &tail);
  if (IsWord(second, "in")) {
    in->op = Op::kFadeIn;
    return true;
  }
  in->op = Op::kFadeOut;
  // fade out (black) or fade out white.
  in->extra = IsWord(tail, "white") ? 1 : 0;
  return IsWord(second, "out");
}

bool ParseMusic(const CommandWords& w, Instruction* in) {
  if (IsWord(w.rest, "end")) {
    in->op = Op::kMusicEnd;
    return true;
  }
  u32 number = 0;
  in->op = Op::kMusic;
  if (!FindOrReport(NameKind::kMusic, w.rest, &number, "unknown music")) {
    return false;
  }
  in->a = static_cast<u16>(number);
  return true;
}

// The commands with one number: wait, sound, call.
template <Op kOp>
bool ParseNumberCommand(const CommandWords& w, Instruction* in) {
  u32 number = 0;
  if (!ParseNumber(w.rest, &number)) return false;
  in->op = kOp;
  in->a = static_cast<u16>(number);
  return true;
}

// give item / take item: the item, then the count (1 without a count).
bool ParseItem(const CommandWords& w, Instruction* in) {
  u32 number = 0;
  u32 count = 1;
  const Span name = SplitLastNumber(w.tail, &count);
  if (!FindOrReport(NameKind::kItem, name, &number, "unknown item")) {
    return false;
  }
  in->op = w.is_first_choice ? Op::kGiveItem : Op::kTakeItem;
  in->a = static_cast<u16>(number);
  in->b = static_cast<u16>(count);
  return true;
}

// give money / take money: the money has 32 bits, in `a` and `b`.
bool ParseMoney(const CommandWords& w, Instruction* in) {
  u32 number = 0;
  if (!ParseNumber(w.tail, &number)) return false;
  in->op = w.is_first_choice ? Op::kGiveMoney : Op::kTakeMoney;
  in->a = static_cast<u16>(number & 0xFFFF);
  in->b = static_cast<u16>(number >> 16);
  return true;
}

// give pokemon / battle wild: the species, then the level (5 without a level).
bool ParseSpeciesLevel(const CommandWords& w, Instruction* in, Op op) {
  u32 number = 0;
  u32 level = 5;
  const Span name = SplitLastNumber(w.tail, &level);
  if (!FindOrReport(NameKind::kSpecies, name, &number, "unknown species")) {
    return false;
  }
  in->op = op;
  in->a = static_cast<u16>(number);
  in->b = static_cast<u16>(level);
  return true;
}

bool ParseGivePokemon(const CommandWords& w, Instruction* in) {
  return ParseSpeciesLevel(w, in, Op::kGivePokemon);
}

bool ParseBattleWild(const CommandWords& w, Instruction* in) {
  return ParseSpeciesLevel(w, in, Op::kBattleWild);
}

// set flag / clear flag.
bool ParseFlag(const CommandWords& w, Instruction* in) {
  u32 number = 0;
  if (!ParseNumber(w.tail, &number)) return false;
  in->op = w.is_first_choice ? Op::kSetFlag : Op::kClearFlag;
  in->a = static_cast<u16>(number);
  return true;
}

// set var / add var: set var 3 10.
bool ParseVariable(const CommandWords& w, Instruction* in) {
  Span variable;
  Span amount;
  SplitWord(w.tail, &variable, &amount);
  u32 number = 0;
  u32 value = 0;
  if (!ParseNumber(variable, &number) || !ParseNumber(amount, &value)) {
    return false;
  }
  in->op = w.is_first_choice ? Op::kSetVariable : Op::kAddVariable;
  in->a = static_cast<u16>(number);
  in->b = static_cast<u16>(value);
  return true;
}

bool ParseBattleTrainer(const CommandWords& w, Instruction* in) {
  u32 number = 0;
  if (!FindOrReport(NameKind::kTrainer, w.tail, &number, "unknown trainer")) {
    return false;
  }
  in->op = Op::kBattleTrainer;
  in->a = static_cast<u16>(number);
  return true;
}

bool ParseWarp(const CommandWords& w, Instruction* in) {
  // warp Route 101, 10, 12, 2, Down
  Span parts[5];
  const u32 count = Split(w.rest, ',', parts, 5);
  u32 number = 0;
  u32 x = 0;
  u32 z = 0;
  u32 height = 0;
  if (count < 4 ||
      !FindOrReport(NameKind::kMap, parts[0], &number, "unknown map") ||
      !ParseNumber(parts[1], &x) || !ParseNumber(parts[2], &z) ||
      !ParseNumber(parts[3], &height)) {
    return false;
  }
  in->op = Op::kWarp;
  in->a = static_cast<u16>(number);
  in->b = static_cast<u16>(x);
  in->c = static_cast<u16>(z);
  in->d = static_cast<u16>(height);
  in->extra = 1; // Down.
  if (count == 5 && !ParseFacing(parts[4], &in->extra)) return false;
  return true;
}

// A command of story.txt: its first word (two words with "/", for example
// "give/take"), its second word (or null), and the function that reads it.
// The words have the normalized form (see Normalize()).
struct CommandSpec {
  const c8* first;
  const c8* second;
  bool (*parse)(const CommandWords& words, Instruction* in);
};

const CommandSpec kCommands[] = {
    {"say", nullptr, ParseSay},
    {"sign", nullptr, ParseSign},
    {"ask", nullptr, ParseAsk},
    {"choose", nullptr, ParseChoose},
    {"if", nullptr, ParseIf},
    {"else", nullptr, ParseSimple<Op::kElse>},
    {"end", nullptr, ParseSimple<Op::kEnd>},
    {"stop", nullptr, ParseSimple<Op::kStop>},
    {"heal", nullptr, ParseSimple<Op::kHeal>},
    {"face", "player", ParseSimple<Op::kFacePlayer>},
    {"fade", nullptr, ParseFade},
    {"music", nullptr, ParseMusic},
    {"wait", nullptr, ParseNumberCommand<Op::kWait>},
    {"sound", nullptr, ParseNumberCommand<Op::kSound>},
    {"call", nullptr, ParseNumberCommand<Op::kCallScript>},
    {"give/take", "item", ParseItem},
    {"give/take", "money", ParseMoney},
    {"give", "pokemon", ParseGivePokemon},
    {"set/clear", "flag", ParseFlag},
    {"set/add", "var", ParseVariable},
    {"battle", "trainer", ParseBattleTrainer},
    {"battle", "wild", ParseBattleWild},
    {"warp", nullptr, ParseWarp},
};

// Returns the place of `word` in `first` ("give/take": 0 for give, 1 for
// take), or -1 when `first` does not have the word.
s32 MatchFirst(const c8* word, const c8* first) {
  s32 index = 0;
  const c8* start = first;
  for (const c8* c = first;; c++) {
    if (*c != '/' && *c != '\0') continue;
    const u32 length = static_cast<u32>(c - start);
    if (std::strlen(word) == length && std::strncmp(word, start, length) == 0) {
      return index;
    }
    if (*c == '\0') return -1;
    start = c + 1;
    index++;
  }
}

// Reads one command. Returns false when the command is not known.
bool ParseCommand(Span line, Instruction* in) {
  Span word;
  Span second;
  CommandWords words;
  SplitWord(line, &word, &words.rest);
  SplitWord(words.rest, &second, &words.tail);
  // The first two words in the normalized form, one time for all the commands.
  c8 first_name[16];
  c8 second_name[16];
  if (!Normalize(word, first_name, sizeof(first_name))) return false;
  if (!Normalize(second, second_name, sizeof(second_name))) second_name[0] = 0;

  for (const CommandSpec& command : kCommands) {
    const s32 index = MatchFirst(first_name, command.first);
    if (index < 0) continue;
    if (command.second != nullptr &&
        std::strcmp(second_name, command.second) != 0) {
      continue;
    }
    words.is_first_choice = index == 0;
    return command.parse(words, in);
  }
  return false;
}

void OnStoryLine(Span line, bool is_section) {
  if (is_section) {
    CloseStory();
    if (line.length == 0) return;
    // [33000]
    u32 id = 0;
    if (!ParseNumber(line, &id) || id < script::kFirstStoryScript ||
        id > script::kLastStoryScript) {
      Report("use a script from 33000 to 33999:", line);
      return;
    }
    if (story_count >= SIZE(stories)) {
      Report("too many scripts:", line);
      return;
    }
    stories[story_count++] = {static_cast<u16>(id),
                              static_cast<u16>(instruction_count), 0};
    open_block_count = 0;
    is_story_open = true;
    sink->OnStory(static_cast<u16>(id));
    return;
  }
  if (!is_story_open) return;
  if (instruction_count >= SIZE(instruction_pool)) {
    Report("too many commands:", line);
    return;
  }
  Instruction in = {};
  in.line = static_cast<u16>(current_line);
  if (!ParseCommand(line, &in)) {
    Report("unknown command:", line);
    return;
  }
  const u16 index = static_cast<u16>(instruction_count);
  if (in.op == Op::kIf) {
    if (open_block_count >= SIZE(open_blocks)) {
      Report("too many if inside each other:", line);
      return;
    }
    open_blocks[open_block_count++] = index;
  } else if (in.op == Op::kElse || in.op == Op::kEnd) {
    if (open_block_count == 0) {
      Report("no if before", line);
      return;
    }
    Instruction& opener = instruction_pool[open_blocks[open_block_count - 1]];
    if (in.op == Op::kElse) {
      if (opener.op != Op::kIf) {
        Report("two else for one if:", line);
        return;
      }
      // A false "if" goes after the "else".
      opener.jump = static_cast<u16>(index + 1);
      open_blocks[open_block_count - 1] = index;
    } else {
      opener.jump = index;
      open_block_count--;
    }
  }
  instruction_pool[instruction_count++] = in;
}

// characters.txt.

struct CharacterState {
  bool is_valid;
  MapId map_id;
} character_state = {false, MapId::kNone};

// Reads "script 33000", "trainer Route 102 Kid 1" or "sight 4" into the
// request. Returns false when the part is not correct.
bool ParseCharacterPart(Span part, Character* request) {
  Span word;
  Span rest;
  SplitWord(Trim(part), &word, &rest);
  u32 number = 0;
  if (IsWord(word, "script")) {
    if (!ParseNumber(rest, &number) || number == 0 || number > 0xFFFF) {
      Report("give a script number:", part);
      return false;
    }
    request->script = static_cast<ScriptId>(number);
    return true;
  }
  if (IsWord(word, "trainer")) {
    if (!FindOrReport(NameKind::kTrainer, rest, &number, "unknown trainer")) {
      return false;
    }
    request->trainer = static_cast<TrainerId>(number);
    return true;
  }
  if (IsWord(word, "sight")) {
    if (!ParseNumber(rest, &number) || number < 1 || number > 10) {
      Report("use a sight from 1 to 10:", part);
      return false;
    }
    request->sight = static_cast<u8>(number);
    return true;
  }
  Report("unknown part:", part);
  return false;
}

void OnCharacterLine(Span line, bool is_section) {
  if (is_section) {
    character_state.is_valid = false;
    if (line.length == 0) return;
    // [Route 101]
    u32 map = 0;
    if (!FindOrReport(NameKind::kMap, line, &map, "unknown map")) return;
    character_state = {true, static_cast<MapId>(map)};
    return;
  }
  if (!character_state.is_valid) {
    ReportOrphan(line);
    return;
  }
  // Youngster, 10, 12, 0, Down, script 33000
  // Youngster, 14, 8, 0, Left, trainer Route 102 Kid 1, sight 4
  Span parts[8];
  const u32 count = Split(line, ',', parts, 8);
  if (count < 6) {
    Report("use model, x, z, height, facing, script N:", line);
    return;
  }
  u32 model = 0;
  u32 x = 0;
  u32 z = 0;
  u32 height = 0;
  u8 facing = 0;
  if (!FindOrReport(NameKind::kModel, parts[0], &model, "unknown model")) {
    return;
  }
  if (!ParseNumber(parts[1], &x) || !ParseNumber(parts[2], &z) ||
      !ParseNumber(parts[3], &height)) {
    Report("give x, z and height after", parts[0]);
    return;
  }
  if (!ParseFacing(parts[4], &facing)) {
    Report("use Up, Down, Left or Right:", parts[4]);
    return;
  }
  Character request = {};
  request.map = character_state.map_id;
  request.model = static_cast<ModelId>(model);
  request.tile_x = static_cast<u16>(x);
  request.tile_z = static_cast<u16>(z);
  request.height = static_cast<u16>(height);
  request.facing = facing;
  request.script = ScriptId::kNone;
  request.trainer = TrainerId::kNone;
  request.sight = 3;
  for (u32 i = 5; i < count; i++) {
    if (!ParseCharacterPart(parts[i], &request)) return;
  }
  if (request.script == ScriptId::kNone &&
      request.trainer == TrainerId::kNone) {
    Report("give a script or a trainer:", line);
    return;
  }
  if (!sink->OnCharacter(request)) {
    Report("too many characters:", parts[0]);
  }
}

// rules.txt.

struct RuleSpec {
  const c8* key; // The normalized key.
  Rule rule;
};

const RuleSpec kRules[] = {
    {"itemsinbattle", Rule::kItemsInBattle},
    {"onemegaevolution", Rule::kOneMegaEvolution},
    {"samecatchrate", Rule::kSameCatchRate},
    {"inversestats", Rule::kInverseStats},
    {"metronomeonly", Rule::kMetronomeOnly},
    {"typehelper", Rule::kTypeHelper},
    {"randomgifts", Rule::kRandomGifts},
    {"noevlimit", Rule::kNoEvLimit},
};

struct MultiplierSpec {
  const c8* key; // The normalized key.
  Multiplier multiplier;
};

const MultiplierSpec kMultipliers[] = {
    {"experience", Multiplier::kExperience},
    {"catchrate", Multiplier::kCatchRate},
    {"money", Multiplier::kMoney},
};

// The largest multiplier, in hundredths (see the Battle page of the menu).
constexpr u32 kMaxMultiplier = 2000;

// "yes", "on", "no" or "off".
bool ParseSwitch(Span value, bool* is_on) {
  if (IsWord(value, "yes") || IsWord(value, "on")) {
    *is_on = true;
    return true;
  }
  if (IsWord(value, "no") || IsWord(value, "off")) {
    *is_on = false;
    return true;
  }
  return false;
}

// "2", "1.5" or "0.25", in hundredths.
bool ParseHundredths(Span value, u32* hundredths) {
  value = Trim(value);
  u32 whole = 0;
  u32 fraction = 0;
  u32 digits = 0;
  bool has_point = false;
  bool has_digit = false;
  for (u32 i = 0; i < value.length; i++) {
    const c8 c = value.text[i];
    if (c == '.' && !has_point) {
      has_point = true;
    } else if (c >= '0' && c <= '9') {
      has_digit = true;
      if (!has_point) {
        whole = whole * 10 + (c - '0');
        if (whole > 100000) return false;
      } else if (digits < 2) {
        fraction = fraction * 10 + (c - '0');
        digits++;
      }
    } else {
      return false;
    }
  }
  if (!has_digit) return false;
  if (digits == 1) fraction *= 10;
  *hundredths = whole * 100 + fraction;
  return true;
}

// "game", or "1/4096": a chance of 1 out of a power of 2 (see ShinyRate).
bool ParseShinyRate(Span value, pokemon::ShinyRate* rate) {
  if (IsWord(value, "game")) {
    *rate = pokemon::ShinyRate::kOff;
    return true;
  }
  Span parts[3];
  u32 one = 0;
  u32 count = 0;
  if (Split(value, '/', parts, 3) != 2 || !ParseNumber(parts[0], &one) ||
      one != 1 || !ParseNumber(parts[1], &count) || count == 0) {
    return false;
  }
  u32 index = 1;
  for (; count > 1; count /= 2) {
    if (count % 2 != 0) return false;
    index++;
  }
  if (index >= static_cast<u32>(pokemon::ShinyRate::kCount)) return false;
  *rate = static_cast<pokemon::ShinyRate>(index);
  return true;
}

void OnRulesLine(Span line, bool is_section) {
  if (is_section) {
    if (line.length != 0) Report("rules.txt has no sections:", line);
    return;
  }
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  for (const RuleSpec& spec : kRules) {
    if (!IsWord(key, spec.key)) continue;
    bool is_on = false;
    if (!ParseSwitch(value, &is_on)) {
      Report("use yes or no:", value);
      return;
    }
    sink->OnRule(spec.rule, is_on);
    return;
  }
  for (const MultiplierSpec& spec : kMultipliers) {
    if (!IsWord(key, spec.key)) continue;
    u32 hundredths = 0;
    if (!ParseHundredths(value, &hundredths) || hundredths > kMaxMultiplier) {
      Report("use a number from 0 to 20:", value);
      return;
    }
    sink->OnMultiplier(spec.multiplier, hundredths);
    return;
  }
  if (IsWord(key, "shinyrate")) {
    pokemon::ShinyRate rate = pokemon::ShinyRate::kOff;
    if (!ParseShinyRate(value, &rate)) {
      Report("use game or 1/4096:", value);
      return;
    }
    sink->OnShinyRate(rate);
  } else if (IsWord(key, "starters")) {
    Span parts[4];
    if (Split(value, ',', parts, 4) != 3) {
      Report("give 3 species:", value);
      return;
    }
    SpeciesId species[3];
    for (u32 k = 0; k < 3; k++) {
      u32 id = 0;
      if (!FindOrReport(NameKind::kSpecies, parts[k], &id,
                        "unknown species")) {
        return;
      }
      species[k] = static_cast<SpeciesId>(id);
    }
    sink->OnStarters(species);
  } else {
    Report("unknown key:", key);
  }
}

// shops.txt.

pokemon::ShopItem shop_item_pool[1024];
u32 shop_item_count = 0;
pokemon::ShopPokemon shop_pokemon_pool[128];
u32 shop_pokemon_count = 0;

struct ShopState {
  bool is_valid;
  bool is_pokemon_shop;
  u32 shop_id;
  u32 first; // The first item or Pokémon of the shop in its pool.
} shop_state = {false, false, kAllShops, 0};

void CloseShop() {
  if (!shop_state.is_valid) return;
  shop_state.is_valid = false;
  if (shop_state.is_pokemon_shop) {
    sink->OnShopPokemons(shop_state.shop_id,
                         shop_pokemon_pool + shop_state.first,
                         shop_pokemon_count - shop_state.first);
  } else {
    sink->OnShopItems(shop_state.shop_id, shop_item_pool + shop_state.first,
                      shop_item_count - shop_state.first);
  }
}

// [shop 12], [all shops], [pokemon shop] or [pokemon shop 12].
bool ParseShopSection(Span line) {
  // The number at the end, if there is one.
  Span name = line;
  u32 shop_id = kAllShops;
  u32 last = line.length;
  while (last > 0 && !IsSpace(line.text[last - 1])) last--;
  u32 number = 0;
  if (last > 0 &&
      ParseNumber(Span{line.text + last, line.length - last}, &number)) {
    shop_id = number;
    name = Trim(Span{line.text, last});
  }
  bool is_pokemon_shop = false;
  if (IsWord(name, "pokemonshop")) {
    is_pokemon_shop = true;
  } else if (shop_id != kAllShops ? !IsWord(name, "shop")
                                  : !IsWord(name, "allshops")) {
    return false;
  }
  shop_state = {true, is_pokemon_shop, shop_id,
                is_pokemon_shop ? shop_pokemon_count : shop_item_count};
  return true;
}

void OnShopLine(Span line, bool is_section) {
  if (is_section) {
    CloseShop();
    if (line.length == 0) return;
    if (!ParseShopSection(line)) {
      Report("use [shop N], [all shops] or [pokemon shop]:", line);
    }
    return;
  }
  if (!shop_state.is_valid) {
    ReportOrphan(line);
    return;
  }
  Span parts[4];
  const u32 count = Split(line, ',', parts, SIZE(parts));
  if (shop_state.is_pokemon_shop) {
    // Eevee, 10, 5000
    u32 species = 0;
    u32 level = 0;
    u32 price = 0;
    if (count != 3) {
      Report("use species, level, price:", line);
      return;
    }
    if (!FindOrReport(NameKind::kSpecies, parts[0], &species,
                      "unknown species")) {
      return;
    }
    if (!ParseNumber(parts[1], &level) || level < 1 || level > 100) {
      Report("use a level from 1 to 100:", parts[1]);
      return;
    }
    if (!ParseNumber(parts[2], &price)) {
      Report("give a price:", parts[2]);
      return;
    }
    if (shop_pokemon_count - shop_state.first >= kMaxShopItems ||
        shop_pokemon_count >= SIZE(shop_pokemon_pool)) {
      Report("too many Pokemon:", parts[0]);
      return;
    }
    shop_pokemon_pool[shop_pokemon_count++] = {
        static_cast<SpeciesId>(species), FormId::kNormal,
        static_cast<u8>(level), price};
    return;
  }
  // Potion, 300
  u32 item = 0;
  u32 price = 0;
  if (count != 2) {
    Report("use item, price:", line);
    return;
  }
  if (!FindOrReport(NameKind::kItem, parts[0], &item, "unknown item")) return;
  if (!ParseNumber(parts[1], &price)) {
    Report("give a price:", parts[1]);
    return;
  }
  if (shop_item_count - shop_state.first >= kMaxShopItems ||
      shop_item_count >= SIZE(shop_item_pool)) {
    Report("too many items:", parts[0]);
    return;
  }
  pokemon::ShopItem& entry = shop_item_pool[shop_item_count++];
  entry.id = static_cast<ItemId>(item);
  entry._0 = 0;
  entry.price = price;
}

// items.txt.

struct ItemState {
  bool is_valid;
  ItemChange change;
} item_state = {};

void CloseItem() {
  if (!item_state.is_valid) return;
  item_state.is_valid = false;
  sink->OnItem(item_state.change);
}

// Reads a number from 0 to `max`.
bool ParseSmallNumber(Span value, u32 max, u32* number) {
  return ParseNumber(value, number) && *number <= max;
}

void OnItemLine(Span line, bool is_section) {
  if (is_section) {
    CloseItem();
    if (line.length == 0) return;
    u32 item = 0;
    if (!FindOrReport(NameKind::kItem, line, &item, "unknown item")) return;
    item_state = {};
    item_state.is_valid = true;
    item_state.change.item = static_cast<ItemId>(item);
    return;
  }
  if (!item_state.is_valid) {
    ReportOrphan(line);
    return;
  }
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  ItemChange& change = item_state.change;
  u32 number = 0;
  if (IsWord(key, "price")) {
    if (!ParseSmallNumber(value, 65535, &number)) {
      Report("use a price from 0 to 65535:", value);
      return;
    }
    change.price = static_cast<u16>(number);
    change.has_price = true;
  } else if (IsWord(key, "flingpower")) {
    if (!ParseSmallNumber(value, 255, &number)) {
      Report("use a number from 0 to 255:", value);
      return;
    }
    change.fling_power = static_cast<u8>(number);
    change.has_fling_power = true;
  } else if (IsWord(key, "hprestore")) {
    if (!ParseSmallNumber(value, 255, &number)) {
      Report("use a number from 0 to 255:", value);
      return;
    }
    change.hp_restore = static_cast<u8>(number);
    change.has_hp_restore = true;
  } else if (IsWord(key, "evs")) {
    Span parts[7];
    if (Split(value, ',', parts, SIZE(parts)) != 6) {
      Report("give 6 EVs:", value);
      return;
    }
    for (u32 k = 0; k < 6; k++) {
      if (!ParseSmallNumber(parts[k], 127, &number)) {
        Report("use EVs from 0 to 127:", parts[k]);
        return;
      }
      change.evs[k] = static_cast<s8>(number);
    }
    change.has_evs = true;
  } else {
    Report("unknown key:", key);
  }
}

// moves.txt.

struct MoveState {
  bool is_valid;
  MoveChange change;
} move_state = {};

void CloseMove() {
  if (!move_state.is_valid) return;
  move_state.is_valid = false;
  sink->OnMove(move_state.change);
}

// "-1", "+1" or "1".
bool ParseSigned(Span value, s32* number) {
  value = Trim(value);
  bool is_negative = false;
  if (value.length > 0 && (value.text[0] == '-' || value.text[0] == '+')) {
    is_negative = value.text[0] == '-';
    value.text++;
    value.length--;
  }
  u32 magnitude = 0;
  if (value.length > 4 || !ParseNumber(value, &magnitude)) return false;
  *number = is_negative ? -static_cast<s32>(magnitude)
                        : static_cast<s32>(magnitude);
  return true;
}

void OnMoveLine(Span line, bool is_section) {
  if (is_section) {
    CloseMove();
    if (line.length == 0) return;
    u32 move = 0;
    if (!FindOrReport(NameKind::kMove, line, &move, "unknown move")) return;
    move_state = {};
    move_state.is_valid = true;
    move_state.change.move = static_cast<MoveId>(move);
    return;
  }
  if (!move_state.is_valid) {
    ReportOrphan(line);
    return;
  }
  Span key;
  Span value;
  if (!SplitKey(line, &key, &value)) {
    Report("use key = value:", line);
    return;
  }
  MoveChange& change = move_state.change;
  u32 number = 0;
  s32 signed_number = 0;
  if (IsWord(key, "type")) {
    if (!FindOrReport(NameKind::kType, value, &number, "unknown type")) return;
    change.type = static_cast<TypeId>(number);
    change.has_type = true;
  } else if (IsWord(key, "category")) {
    if (IsWord(value, "physical")) {
      change.category = DamageCategory::kPhysical;
    } else if (IsWord(value, "special")) {
      change.category = DamageCategory::kSpecial;
    } else if (IsWord(value, "status")) {
      change.category = DamageCategory::kStatus;
    } else {
      Report("use physical, special or status:", value);
      return;
    }
    change.has_category = true;
  } else if (IsWord(key, "power")) {
    if (!ParseSmallNumber(value, 255, &number)) {
      Report("use a power from 0 to 255:", value);
      return;
    }
    change.power = static_cast<u8>(number);
    change.has_power = true;
  } else if (IsWord(key, "accuracy")) {
    if (IsWord(value, "nevermisses")) {
      number = 101;
    } else if (!ParseSmallNumber(value, 100, &number)) {
      Report("use 0 to 100, or never misses:", value);
      return;
    }
    change.accuracy = static_cast<u8>(number);
    change.has_accuracy = true;
  } else if (IsWord(key, "pp")) {
    if (!ParseSmallNumber(value, 64, &number) || number == 0) {
      Report("use PP from 1 to 64:", value);
      return;
    }
    change.pp = static_cast<u8>(number);
    change.has_pp = true;
  } else if (IsWord(key, "priority")) {
    if (!ParseSigned(value, &signed_number) || signed_number < -7 ||
        signed_number > 5) {
      Report("use a priority from -7 to 5:", value);
      return;
    }
    change.priority = static_cast<s8>(signed_number);
    change.has_priority = true;
  } else {
    Report("unknown key:", key);
  }
}

} // namespace

void ReadEncounters(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "encounters.txt", sink, OnEncounterLine);
}

void ReadTrainers(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "trainers.txt", sink, OnTrainerLine);
}

void ReadSpecies(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "species.txt", sink, OnSpeciesLine);
}

void ReadStory(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "story.txt", sink, OnStoryLine);
}

void ReadCharacters(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "characters.txt", sink, OnCharacterLine);
}

void ReadRules(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "rules.txt", sink, OnRulesLine);
}

void ReadShops(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "shops.txt", sink, OnShopLine);
}

void ReadItems(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "items.txt", sink, OnItemLine);
}

void ReadMoves(const c8* text, u32 size, Sink& sink) {
  ReadLines(text, size, "moves.txt", sink, OnMoveLine);
}

const Story* FindStory(u16 id) {
  const Story* story = nullptr;
  for (u32 i = 0; i < story_count; i++) {
    if (stories[i].id == id) story = &stories[i];
  }
  return story;
}

const Instruction& GetInstruction(u32 index) {
  return instruction_pool[index < instruction_count ? index : 0];
}

const c8* GetCommandName(Op op) {
  // The same order as Op.
  static const c8* const kNames[] = {
      "say", "sign", "ask", "choose", "if", "else", "end", "stop",
      "face player", "give item", "take item", "give pokemon", "give money",
      "take money", "set flag", "clear flag", "set var", "add var",
      "battle trainer", "battle wild", "warp", "heal", "fade out", "fade in",
      "music", "music end", "wait", "sound", "call",
  };
  static_assert(SIZE(kNames) == static_cast<u32>(Op::kCallScript) + 1,
                "one name for each command");
  const u32 index = static_cast<u32>(op);
  return index < SIZE(kNames) ? kNames[index] : "?";
}

namespace {
template <typename T, u32 N>
bool IsIn(const void* address, const T (&pool)[N]) {
  const u8* byte = static_cast<const u8*>(address);
  const u8* start = reinterpret_cast<const u8*>(pool);
  return byte >= start && byte < start + sizeof(pool);
}
} // namespace

bool IsReaderMemory(const void* address) {
  return IsIn(address, slot_pool) || IsIn(address, move_pool) ||
         IsIn(address, step_pool) || IsIn(address, text_pool) ||
         IsIn(address, story_text_pool) || IsIn(address, shop_item_pool) ||
         IsIn(address, shop_pokemon_pool);
}

void Clear() {
  text_pool_size = 0;
  slot_count = 0;
  team_count = 0;
  move_count = 0;
  step_count = 0;
  instruction_count = 0;
  story_count = 0;
  story_text_size = 0;
  open_block_count = 0;
  is_story_open = false;
  is_trainer_open = false;
  shop_item_count = 0;
  shop_pokemon_count = 0;
  shop_state.is_valid = false;
  item_state.is_valid = false;
  move_state.is_valid = false;
  new_species_count = 0;
  new_trainer_count = 0;
  new_class_count = 0;
  is_class_open = false;
}

} // namespace maker

namespace maker {
namespace {

// Finds a name (or a number). `with_new_species`: also the names of the new
// species of species.txt.
bool FindName(NameKind kind, Span span, bool with_new_species, u32* value) {
  span = Trim(span);
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
  if (!with_new_species) return false;
  if (kind == NameKind::kTrainer) return FindNewTrainer(name, value);
  if (kind == NameKind::kTrainerClass) return FindNewClass(name, value);
  return kind == NameKind::kSpecies && FindNewSpecies(name, value);
}

bool FindGameName(NameKind kind, Span span, u32* value) {
  return FindName(kind, span, false, value);
}

} // namespace
} // namespace maker

void DataNames::Release() {
#ifdef SANGO_NAME_FILES
  maker::ReleaseTables();
#endif
}

bool DataNames::IsAvailable() {
  return maker::GetTable(NameKind::kSpecies).count != 0;
}

bool DataNames::Find(NameKind kind, const c8* text, u32 length, u32* value) {
  return maker::FindName(kind, maker::Span{text, length}, true, value);
}

const c8* DataNames::GetName(NameKind kind, u32 value) {
  using namespace maker;
  const NameTable table = GetTable(kind);
  for (u32 i = 0; i < table.count; i++) {
    if (table.names[i].value == value) return table.names[i].name;
  }
  if (kind == NameKind::kTrainerClass) {
    for (u32 i = 0; i < new_class_count; i++) {
      if (new_class_names[i].id == value) return new_class_names[i].name;
    }
    return nullptr;
  }
  if (kind == NameKind::kTrainer) {
    for (u32 i = 0; i < new_trainer_count; i++) {
      if (new_trainer_names[i].id == value) return new_trainer_names[i].name;
    }
    return nullptr;
  }
  if (kind != NameKind::kSpecies) return nullptr;
  for (u32 i = 0; i < new_species_count; i++) {
    if (new_species_names[i].id == value) return new_species_names[i].name;
  }
  return nullptr;
}

} // namespace core
