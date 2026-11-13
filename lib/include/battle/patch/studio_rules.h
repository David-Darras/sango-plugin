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
 * @file studio_rules.h
 * @brief The rules of the new abilities, moves and items of Sango Studio.
 *
 * A rule is "when this happens, if all these conditions are true, do these
 * actions". The studio writes the rules in the file of an ability, of a move or
 * of an item, and the plugin runs them with one reaction function that it
 * writes once: the user writes no code. This file has the parts that the three
 * features share: the layout of the rules in a file and the code that runs
 * them in a battle.
 *
 * The layout of the rules (all the numbers are little-endian). A rule is a
 * 16-bit moment (a value of battle::MomentKind), the number of conditions (1
 * byte), the number of actions (1 byte), the conditions (3 bytes each) and the
 * actions (7 bytes each). A condition is its kind (1 byte) and a signed 16-bit
 * argument. An action is its kind (1 byte), its target (1 byte), two signed
 * 16-bit arguments `a` and `b`, and its flags (1 byte; bit 0 shows the banner
 * of the ability).
 *
 * @see battle/patch/studio_abilities.h
 */

#pragma once

#include "common.h"
#include "battle/native/uid.h"
#include "system/constant/language.h"

namespace battle {
class Controller;

namespace studio {

/// The limits of one ability, move or item. The studio uses the same limits.
constexpr u32 kMaxRules = 8;
constexpr u32 kMaxConditions = 6;
constexpr u32 kMaxActions = 6;
/// The number of languages in the files (see core::Language).
constexpr u32 kLanguageCount = 8;

/// A test of a rule.
struct Condition {
  s16 arg;
  u8 kind;
};

/// A thing that a rule does.
struct Action {
  s16 a;
  s16 b;
  u8 kind;
  u8 target;
  u8 banner;
};

/// A rule: a moment, the conditions that must all be true, and the actions.
struct Rule {
  u16 moment;
  u8 condition_count;
  u8 action_count;
  Condition conditions[kMaxConditions];
  Action actions[kMaxActions];
};

/// Reads the bytes of a file one part after the other.
class Reader {
public:
  Reader(const u8* bytes, u32 size) : bytes_(bytes), size_(size) {}

  /// Skips `count` bytes. Returns false when the file is too short.
  bool Skip(u32 count);
  /// Returns `count` bytes, or null when the file is too short.
  const u8* Take(u32 count);
  /// Reads a text: a 16-bit length (in 16-bit units), then the units. Copies at
  /// most `capacity - 1` units and a zero when `out` is not null. Returns
  /// false when the file is too short.
  bool Text(c16* out, u32 capacity);

private:
  const u8* bytes_;
  u32 size_;
  u32 at_ = 0;
};

/// Returns the position of the language of the game in the files (the order
/// of the files: Japanese, Japanese with kanji, English, French, Italian,
/// German, Spanish, Korean).
u32 LanguageIndex(sys::Language language);

/// Reads a 16-bit little-endian number.
inline u16 Read16(const u8* at) { return static_cast<u16>(at[0] | (at[1] << 8)); }

/// Reads the texts of a file for the language of the game: for each of the
/// eight languages, `text_count` texts. `outs[i]` and `capacities[i]` are the
/// buffer of the text number `i` for the language of the game. Returns false
/// when the file is too short.
bool ReadTexts(Reader* reader, u32 text_count, c16* const* outs,
               const u32* capacities);

/// Reads `count` rules. Returns false for a file that is cut or that has a
/// value that this version does not know.
bool ReadRules(Reader* reader, u32 count, Rule* rules);

/// Runs a rule of an ability, of a move or of an item. `rule_index` is the
/// place of the rule: the listener keeps one bit for each rule in the first
/// number of `local_state`, for the condition "only the first time".
void Execute(const Rule& rule, u32 rule_index, Controller* controller,
             UID owner, s32* local_state);

/// Writes the full path of a file of the studio: `folder`, the number `id`,
/// then `suffix` (for example "sdmc:/sango-studio/move/custom/", 880, ".bin").
void MakePath(const c16* folder, u32 id, const c16* suffix, c16* out);

} // namespace studio
} // namespace battle
