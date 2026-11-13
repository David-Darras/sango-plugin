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
 * @file studio_types.cc
 * @brief The new types and the type chart that Sango Studio makes.
 *
 * The declarations and the layout of the files are in
 * battle/patch/studio_types.h.
 */

#include "battle/patch/studio_types.h"

#include <cstring>

#include "battle/constant/type_multiplier.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/type_chart.h"
#include "core/problems.h"
#include "pokemon/constant/type.h"
#include "system/constant/language.h"
#include "system/native/core.h"
#include "system/native/file.h"

namespace battle {
namespace {

constexpr u32 kGameTypeCount = 18;
constexpr u32 kNoType = 18;
constexpr u32 kTypeCount = StudioTypes::kLastId - StudioTypes::kFirstId + 1;
constexpr u32 kPathSize = 56;
/// The columns of types.csv with all the languages.
constexpr u32 kColumnCount = 10;
constexpr u32 kEnglishColumn = 1;
constexpr u32 kMaxFields = kColumnCount;
constexpr u32 kMaxListSize = 4096;
/// The chart file: 12 bytes, then 32 x 32 cells.
constexpr u32 kChartSide = 32;
constexpr u32 kChartHeader = 12;
constexpr u32 kChartSize = kChartHeader + kChartSide * kChartSide;

const c16 kFolder[] = u"sdmc:/sango-studio/type/";

/// A new type of the studio: the name in the language of the game, and the
/// path of its icon.
struct Entry {
  bool used;
  u8 colour;
  c16 name[StudioTypes::kNameSize];
  c16 icon_path[kPathSize];
};

Entry entries[kTypeCount];
/// The icons that the studio gives to the types of the game.
c16 game_icon_paths[kGameTypeCount][kPathSize];
bool is_game_icon_set[kGameTypeCount];

u8 list_buffer[kMaxListSize + 1];
u8 chart_buffer[kChartSize];
sys::Language loaded_language = sys::Language::kNone;

/// Returns the column of types.csv for the language of the game.
u32 LanguageColumn(sys::Language language) {
  switch (language) {
  case sys::Language::kJapanese: return 3;
  case sys::Language::kFrench: return 5;
  case sys::Language::kItalian: return 6;
  case sys::Language::kGerman: return 7;
  case sys::Language::kSpanish: return 8;
  case sys::Language::kKorean: return 9;
  default: return kEnglishColumn;
  }
}

/// Writes "sdmc:/sango-studio/type/icon/<id>.bclim".
void MakeIconPath(u32 id, c16* out) {
  static const c16 kIcon[] = u"icon/";
  static const c16 kSuffix[] = u".bclim";
  u32 at = 0;
  for (const c16* c = kFolder; *c != 0; c++) out[at++] = *c;
  for (const c16* c = kIcon; *c != 0; c++) out[at++] = *c;
  c16 digits[4];
  u32 count = 0;
  do {
    digits[count++] = static_cast<c16>(u'0' + id % 10);
    id /= 10;
  } while (id != 0);
  while (count > 0) out[at++] = digits[--count];
  for (const c16* c = kSuffix; *c != 0; c++) out[at++] = *c;
  out[at] = 0;
}

/// Writes the path of one of the files of the folder (for example "types.csv").
void MakeFilePath(const c16* name, c16* out) {
  u32 at = 0;
  for (const c16* c = kFolder; *c != 0; c++) out[at++] = *c;
  for (const c16* c = name; *c != 0; c++) out[at++] = *c;
  out[at] = 0;
}

bool FileExists(const c16* path) {
  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) return false;
  file.Close();
  return true;
}

/// Reads a number at the start of a field (`length` characters).
bool ReadNumber(const c8* text, u32 length, u32* value) {
  if (length == 0 || length > 5) return false;
  u32 out = 0;
  for (u32 i = 0; i < length; i++) {
    if (text[i] < '0' || text[i] > '9') return false;
    out = out * 10 + static_cast<u32>(text[i] - '0');
  }
  *value = out;
  return true;
}

/// Converts a UTF-8 text of `length` bytes to 16-bit units. At most
/// `capacity - 1` units are written, then a zero.
void Utf8ToUnits(const c8* text, u32 length, c16* out, u32 capacity) {
  u32 written = 0;
  u32 at = 0;
  while (at < length && written + 2 < capacity) {
    const u8 first = static_cast<u8>(text[at]);
    u32 code = first;
    u32 extra = 0;
    if (first >= 0xF0) {
      code = first & 0x07;
      extra = 3;
    } else if (first >= 0xE0) {
      code = first & 0x0F;
      extra = 2;
    } else if (first >= 0xC0) {
      code = first & 0x1F;
      extra = 1;
    }
    at++;
    for (u32 i = 0; i < extra && at < length; i++, at++) {
      code = (code << 6) | (static_cast<u8>(text[at]) & 0x3F);
    }
    if (code >= 0x10000) {
      code -= 0x10000;
      out[written++] = static_cast<c16>(0xD800 + (code >> 10));
      out[written++] = static_cast<c16>(0xDC00 + (code & 0x3FF));
    } else {
      out[written++] = static_cast<c16>(code);
    }
  }
  out[written] = 0;
}

/// A line of the list, cut into its fields.
struct Fields {
  const c8* text[kMaxFields];
  u32 length[kMaxFields];
  u32 count;
};

Fields Split(const c8* line, u32 length) {
  Fields fields;
  fields.count = 0;
  u32 start = 0;
  for (u32 i = 0; i <= length; i++) {
    if (i == length || line[i] == ',') {
      if (fields.count < kMaxFields) {
        fields.text[fields.count] = line + start;
        fields.length[fields.count] = i - start;
        fields.count++;
      }
      start = i + 1;
    }
  }
  return fields;
}

/// Puts a type of the list in the list of the game: the first time it adds the
/// type, then it updates it.
void Publish(u32 id, Entry* entry) {
  TypeSpec spec;
  spec.id = static_cast<TypeId>(id);
  spec.name = entry->name;
  spec.colour_type = static_cast<TypeId>(entry->colour);
  spec.icon_path = entry->icon_path;
  if (GameExtension::FindType(id) != nullptr) {
    GameExtension::UpdateType(spec);
  } else {
    GameExtension::AddType(spec);
  }
}

/// Reads one line of the list. Gives false for a wrong line.
void ReadLine(const c8* line, u32 length, u32 columns, u32 language_column) {
  const Fields fields = Split(line, length);
  if (fields.count != columns) {
    core::Problems::Add(u"studio: a line of types.csv has a wrong size");
    return;
  }
  u32 id = 0;
  u32 colour = 0;
  if (!ReadNumber(fields.text[0], fields.length[0], &id) ||
      id < StudioTypes::kFirstId || id > StudioTypes::kLastId ||
      fields.length[1] == 0 ||
      !ReadNumber(fields.text[2], fields.length[2], &colour) ||
      colour >= kGameTypeCount) {
    core::Problems::Add(u"studio: a line of types.csv has a wrong value");
    return;
  }
  Entry& entry = entries[id - StudioTypes::kFirstId];
  entry.used = true;
  entry.colour = static_cast<u8>(colour);
  // The language column; the English name when it is empty.
  u32 column = kEnglishColumn;
  if (columns == kColumnCount && fields.length[language_column] != 0) {
    column = language_column;
  }
  Utf8ToUnits(fields.text[column], fields.length[column], entry.name,
              StudioTypes::kNameSize);
  MakeIconPath(id, entry.icon_path);
  Publish(id, &entry);
}

/// Reads types.csv.
void LoadList() {
  c16 path[kPathSize];
  MakeFilePath(u"types.csv", path);
  const u32 size = sys::File::ReadAll(path, list_buffer, kMaxListSize);
  if (size == 0) return;
  list_buffer[size] = 0;
  const c8* text = reinterpret_cast<const c8*>(list_buffer);
  u32 at = 0;
  // A byte order mark.
  if (size >= 3 && list_buffer[0] == 0xEF && list_buffer[1] == 0xBB &&
      list_buffer[2] == 0xBF) {
    at = 3;
  }
  const u32 language_column =
      LanguageColumn(sys::Core::GetInstance().GetLanguage());
  u32 columns = 0;
  bool is_first = true;
  while (at < size) {
    u32 end = at;
    while (end < size && text[end] != '\n') end++;
    u32 length = end - at;
    if (length > 0 && text[at + length - 1] == '\r') length--;
    if (length > 0) {
      if (is_first) {
        is_first = false;
        columns = Split(text + at, length).count;
        if (columns != 3 && columns != kColumnCount) {
          core::Problems::Add(u"studio: the header of types.csv is wrong");
          return;
        }
      } else {
        ReadLine(text + at, length, columns, language_column);
      }
    }
    at = end + 1;
  }
}

/// Reads chart.bin and gives its cells to the type chart.
void LoadChart() {
  c16 path[kPathSize];
  MakeFilePath(u"chart.bin", path);
  const u32 size = sys::File::ReadAll(path, chart_buffer, kChartSize);
  if (size == 0) return;
  u32 version = 0;
  u32 side = 0;
  std::memcpy(&version, chart_buffer + 4, 4);
  std::memcpy(&side, chart_buffer + 8, 4);
  if (size < kChartSize || std::memcmp(chart_buffer, "TCHT", 4) != 0 ||
      version != 1 || side != kChartSide) {
    core::Problems::Add(u"studio: type chart file is not valid");
    return;
  }
  for (u32 attacking = 0; attacking < kChartSide; attacking++) {
    for (u32 defending = 0; defending < kChartSide; defending++) {
      if (attacking == kNoType || defending == kNoType) continue;
      const u8 value =
          chart_buffer[kChartHeader + attacking * kChartSide + defending];
      if (value != 0 && value != 2 && value != 4 && value != 8) continue;
      TypeChart::Set(static_cast<TypeId>(attacking),
                     static_cast<TypeId>(defending),
                     static_cast<TypeMultiplier>(value));
    }
  }
}

/// Looks for the icons that the studio gives to the types of the game.
void LoadGameIcons() {
  for (u32 id = 0; id < kGameTypeCount; id++) {
    if (is_game_icon_set[id]) continue;
    MakeIconPath(id, game_icon_paths[id]);
    if (!FileExists(game_icon_paths[id])) continue;
    is_game_icon_set[id] = true;
    TypeSpec spec;
    spec.id = static_cast<TypeId>(id);
    spec.name = nullptr;
    spec.colour_type = static_cast<TypeId>(id);
    spec.icon_path = game_icon_paths[id];
    GameExtension::AddType(spec);
  }
}

} // namespace

void StudioTypes::LoadAll() {
  loaded_language = sys::Core::GetInstance().GetLanguage();
  LoadList();
  LoadGameIcons();
  LoadChart();
}

void StudioTypes::Apply(const c8* path) {
  if (std::strcmp(path, "type/types.csv") == 0) {
    LoadList();
  } else if (std::strcmp(path, "type/chart.bin") == 0) {
    LoadChart();
  }
  // The icons are read by the game when it needs them (see renderer/
  // type_icon.cc): a changed icon shows after the game restarts.
}

void StudioTypes::RefreshLanguage() {
  const sys::Language language = sys::Core::GetInstance().GetLanguage();
  if (language == loaded_language) return;
  loaded_language = language;
  LoadList();
}

} // namespace battle
