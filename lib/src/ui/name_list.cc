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
 * @file name_list.cc
 * @brief The names of the values of an entry, and the filter of the menu.
 *
 * The declarations are in ui/name_list.h.
 */

#include "ui/name_list.h"

#include <cstring>

#include "core/constant/archive_id.h"
#include "core/game_file.h"
#include "core/utils.h"
#include "pokemon/address.h"
#include "pokemon/native/item_data.h"
#include "system/native/core.h"
#include "system/native/string.h"
#include "ui/page_item.h"

namespace ui {
namespace {
// The names that the menu keeps for each NameSource of the game.
struct GameNames {
  c16* chars = nullptr; ///< All the names, one after the other.
  u32* starts = nullptr; ///< The start of each name, then the end.
  u32 count = 0;
  Language language = Language::kNone; ///< The language of the names.
  bool is_tried = false; ///< The menu read the file for this language.
};

// The game names: kSpecies, kItem, kMove, kAbility.
constexpr u32 kGameSourceCount = 4;
GameNames g_names[kGameSourceCount];

// The text files of the names in the text archive of a language (ORAS).
const u16 kNameFiles[kGameSourceCount] = {98, 116, 15, 37};

// A text file of the game: the strings are XOR-coded with a key that
// changes for each string and each character.
constexpr u16 kStringKey = 0x7C89;
constexpr u16 kStringKeyStep = 0x2983;
// A string that starts with this character has a variable (no fixed name).
constexpr c16 kVariableStart = 0x10;

s32 GetGameIndex(NameSource source) {
  switch (source) {
    case NameSource::kSpecies:
      return 0;
    case NameSource::kItem:
      return 1;
    case NameSource::kMove:
      return 2;
    case NameSource::kAbility:
      return 3;
    default:
      return -1;
  }
}

u16 ReadU16(const u8* data) { return data[0] | (data[1] << 8); }

u32 ReadU32(const u8* data) {
  return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

void Release(GameNames& names) {
  delete[] names.chars;
  delete[] names.starts;
  names.chars = nullptr;
  names.starts = nullptr;
  names.count = 0;
}

// Reads the names of a source from the text archive of the language of the
// game. Returns false when the names are not available.
bool LoadNames(u32 index) {
#ifdef GAME_XY
  (void)index;
  return false;
#else
  GameNames& names = g_names[index];
  const Language language = sys::Core::GetInstance().GetLanguage();
  if (names.is_tried && names.language == language) {
    return names.chars != nullptr;
  }
  Release(names);
  names.language = language;
  names.is_tried = true;

  ArchiveId archive;
  switch (language) {
    case Language::kJapanese:
      archive = ArchiveId::kGameTextJapanese;
      break;
    case Language::kEnglish:
      archive = ArchiveId::kGameTextEnglish;
      break;
    case Language::kFrench:
      archive = ArchiveId::kGameTextFrench;
      break;
    case Language::kItalian:
      archive = ArchiveId::kGameTextItalian;
      break;
    case Language::kGerman:
      archive = ArchiveId::kGameTextGerman;
      break;
    case Language::kSpanish:
      archive = ArchiveId::kGameTextSpanish;
      break;
    case Language::kKorean:
      archive = ArchiveId::kGameTextKorean;
      break;
    default:
      return false;
  }

  u32 size = 0;
  auto* file = core::GameFile::Read(archive, kNameFiles[index], false, &size);
  if (file == nullptr) return false;

  // The header: the number of languages, the number of strings, then the
  // offset of the block of the strings. The block: its size, then an
  // offset (from the block) and a length for each string.
  bool is_loaded = false;
  if (size >= 0x10) {
    const u32 count = ReadU16(file + 2);
    const u32 block = ReadU32(file + 0xC);
    u32 total = 0;
    bool is_valid = block + 4 + count * 8 <= size;
    for (u32 i = 0; is_valid && i < count; i++) {
      const u8* info = file + block + 4 + i * 8;
      const u32 offset = ReadU32(info);
      const u32 length = ReadU16(info + 4);
      if (block + offset + length * 2 > size) is_valid = false;
      total += length;
    }
    if (is_valid) {
      names.chars = new c16[total + 1];
      names.starts = new u32[count + 1];
    }
    if (names.chars != nullptr && names.starts != nullptr) {
      u32 cursor = 0;
      for (u32 i = 0; i < count; i++) {
        const u8* info = file + block + 4 + i * 8;
        const u8* text = file + block + ReadU32(info);
        const u32 length = ReadU16(info + 4);
        u16 key = (u16)(kStringKey + kStringKeyStep * i);
        names.starts[i] = cursor;
        const u32 start = cursor;
        for (u32 j = 0; j < length; j++) {
          const c16 c = ReadU16(text + j * 2) ^ key;
          key = (u16)((key << 3) | (key >> 13));
          if (c == 0) break;
          names.chars[cursor++] = c;
        }
        // A name with a variable is not a fixed name.
        if (cursor > start && names.chars[start] == kVariableStart) {
          cursor = start;
        }
      }
      names.starts[count] = cursor;
      names.count = count;
      is_loaded = true;
    } else {
      Release(names);
    }
  }
  delete[] file;
  return is_loaded;
#endif
}

// Copies a name of the game texts. Returns false when there is no name.
bool CopyGameName(u32 index, u32 value, c16* out, u32 capacity) {
  if (!LoadNames(index)) return false;
  const GameNames& names = g_names[index];
  if (value >= names.count) return false;
  const u32 start = names.starts[value];
  const u32 end = names.starts[value + 1];
  if (end <= start) return false;
  u32 length = 0;
  for (u32 i = start; i < end && length + 1 < capacity; i++) {
    out[length++] = names.chars[i];
  }
  out[length] = 0;
  return true;
}

// Asks the game for a name (XY, or when the texts are not available).
bool AskGameName(NameSource source, u32 value, c16* out, u32 capacity) {
  switch (source) {
    case NameSource::kSpecies:
      ((void (*)(String*, u16))pokemon::address::kGetSpeciesName)(
          String::GetTmpStr(), (u16)value);
      break;
    case NameSource::kMove:
      ((void (*)(u16, String*))pokemon::address::kGetMoveName)(
          (u16)value, String::GetTmpStr());
      break;
    case NameSource::kAbility:
      ((void (*)(String*, u8))pokemon::address::kGetAbilityName)(
          String::GetTmpStr(), (u8)value);
      break;
    case NameSource::kItem: {
      pokemon::ItemData item((ItemId)value);
      item.GetName(String::GetTmpStr());
      break;
    }
    default:
      return false;
  }
  const c16* text = String::GetTmpBuf();
  u32 length = 0;
  while (text[length] != 0 && length + 1 < capacity) {
    out[length] = text[length];
    length++;
  }
  out[length] = 0;
  return length != 0;
}

// Converts a UTF-8 text into UTF-16.
void FromUtf8(const c8* text, c16* out, u32 capacity) {
  u32 length = 0;
  const u8* bytes = (const u8*)text;
  while (*bytes != 0 && length + 1 < capacity) {
    c16 c = *bytes++;
    if ((c & 0xE0) == 0xC0 && *bytes != 0) {
      c = ((c & 0x1F) << 6) | (*bytes++ & 0x3F);
    } else if ((c & 0xF0) == 0xE0 && bytes[0] != 0 && bytes[1] != 0) {
      c = ((c & 0x0F) << 12) | ((bytes[0] & 0x3F) << 6) | (bytes[1] & 0x3F);
      bytes += 2;
    }
    out[length++] = c;
  }
  out[length] = 0;
}

// The letter without its accent of the characters U+00C0 to U+00FF, in
// small letters. 0: the character stays. '2': two letters (Æ, ß).
const c8 kLatin1Letters[64] = {
    'a', 'a', 'a', 'a', 'a', 'a', '2', 'c', 'e', 'e', 'e', 'e', 'i',
    'i', 'i', 'i', 'd', 'n', 'o', 'o', 'o', 'o', 'o', 0,   'o', 'u',
    'u', 'u', 'u', 'y', 0,   '2', 'a', 'a', 'a', 'a', 'a', 'a', '2',
    'c', 'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i', 'd', 'n', 'o', 'o',
    'o', 'o', 'o', 0,   'o', 'u', 'u', 'u', 'u', 'y', 0,   'y'};

// The number of a value, as a text.
void WriteNumber(u32 value, c16* out, u32 capacity) {
  c16 digits[12];
  u32 count = 0;
  do {
    digits[count++] = u'0' + value % 10;
    value /= 10;
  } while (value != 0 && count < SIZE(digits));
  u32 length = 0;
  while (count > 0 && length + 1 < capacity) out[length++] = digits[--count];
  out[length] = 0;
}

// The rank of a name for a query: 0 (the name starts with the query), 1 (a
// word starts with the query), 2 (the query is in the name), or -1.
s32 Rank(const c16* folded_name, const c16* folded_query) {
  const c16* found = nullptr;
  s32 best = -1;
  for (const c16* start = folded_name; *start != 0; start++) {
    u32 i = 0;
    while (folded_query[i] != 0 && start[i] == folded_query[i]) i++;
    if (folded_query[i] != 0) continue;
    found = start;
    s32 rank = 2;
    if (found == folded_name) {
      rank = 0;
    } else {
      const c16 before = found[-1];
      if (before == u' ' || before == u'-' || before == u'\'' ||
          before == u'.') {
        rank = 1;
      }
    }
    if (best < 0 || rank < best) best = rank;
    if (best == 0) break;
  }
  return best;
}

bool IsNumber(const c16* text) {
  if (text[0] == 0) return false;
  for (; *text != 0; text++) {
    if (*text < u'0' || *text > u'9') return false;
  }
  return true;
}
} // namespace

NameSource NameList::GetSource(const PageItem& entry) {
  if (entry.GetArraySize() != 0) return NameSource::kArray;
  switch (entry.GetType()) {
    case kTypeSpecies:
      return NameSource::kSpecies;
    case kTypeItem:
      return NameSource::kItem;
    case kTypeMove:
      return NameSource::kMove;
    case kTypeAbility:
      return NameSource::kAbility;
    default:
      return NameSource::kNone;
  }
}

bool NameList::GetName(const PageItem& entry, u32 value, c16* out,
                       u32 capacity) {
  if (capacity == 0) return false;
  const NameSource source = GetSource(entry);
  if (source == NameSource::kArray) {
    if (value < entry.GetArraySize()) {
      FromUtf8(entry.GetArray()[value], out, capacity);
      return true;
    }
  } else if (source != NameSource::kNone) {
    const s32 index = GetGameIndex(source);
    if (CopyGameName(index, value, out, capacity)) return true;
    // XY, or no text file: ask the game (slower).
    if (g_names[index].chars == nullptr &&
        AskGameName(source, value, out, capacity)) {
      return true;
    }
  }
  WriteNumber(value, out, capacity);
  return false;
}

u32 NameList::Fold(const c16* text, c16* out, u32 capacity) {
  u32 length = 0;
  for (; *text != 0 && length + 2 < capacity; text++) {
    c16 c = *text;
    if (c >= u'A' && c <= u'Z') {
      c += u'a' - u'A';
    } else if (c >= 0xC0 && c <= 0xFF) {
      const c8 letter = kLatin1Letters[c - 0xC0];
      if (letter == '2') {
        // Æ, æ: "ae". ß: "ss".
        const bool is_sharp_s = c == 0xDF;
        out[length++] = is_sharp_s ? u's' : u'a';
        c = is_sharp_s ? u's' : u'e';
      } else if (letter != 0) {
        c = letter;
      }
    } else if (c == 0x152 || c == 0x153) {
      out[length++] = u'o'; // Œ, œ: "oe".
      c = u'e';
    } else if (c == 0x2019 || c == 0x2018) {
      c = u'\''; // The typographic apostrophes.
    }
    out[length++] = c;
  }
  out[length] = 0;
  return length;
}

u32 NameList::Filter(const PageItem& entry, u32 count, const c16* query,
                     const u16* first, u32 first_count, u16* results,
                     u32 capacity) {
  if (count > kMaxResults) count = kMaxResults;
  if (capacity > kMaxResults) capacity = kMaxResults;
  // One bit for each value that is in the results.
  u32 used[kMaxResults / 32];
  memset(used, 0, sizeof(used));
  u32 result_count = 0;
  auto add = [&](u32 value) {
    if (value >= count || result_count >= capacity) return;
    if (used[value / 32] & (1u << (value % 32))) return;
    used[value / 32] |= 1u << (value % 32);
    results[result_count++] = (u16)value;
  };

  c16 folded_query[32];
  Fold(query, folded_query, SIZE(folded_query));
  if (folded_query[0] == 0) {
    for (u32 i = 0; i < first_count; i++) add(first[i]);
    for (u32 value = 0; value < count; value++) add(value);
    return result_count;
  }

  // A number: the values that start with this number come first.
  if (IsNumber(folded_query)) {
    c16 number[12];
    for (u32 value = 0; value < count; value++) {
      WriteNumber(value, number, SIZE(number));
      u32 i = 0;
      while (folded_query[i] != 0 && number[i] == folded_query[i]) i++;
      if (folded_query[i] == 0) add(value);
    }
  }

  // The ranks of the values: 0, 1 or 2 (two bits each), 3 for no match.
  static u8 ranks[kMaxResults];
  c16 name[64];
  c16 folded_name[80];
  for (u32 value = 0; value < count; value++) {
    GetName(entry, value, name, SIZE(name));
    Fold(name, folded_name, SIZE(folded_name));
    const s32 rank = Rank(folded_name, folded_query);
    ranks[value] = rank < 0 ? 3 : (u8)rank;
  }
  for (u8 rank = 0; rank < 3; rank++) {
    for (u32 i = 0; i < first_count; i++) {
      if (first[i] < count && ranks[first[i]] == rank) add(first[i]);
    }
    for (u32 value = 0; value < count; value++) {
      if (ranks[value] == rank) add(value);
    }
  }
  return result_count;
}
} // namespace ui
