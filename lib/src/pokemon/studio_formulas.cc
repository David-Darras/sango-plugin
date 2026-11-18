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
 * @file studio_formulas.cc
 * @brief The formulas of the statistics, the natures and the shiny chance of Sango Studio.
 *
 * The declarations and the layout of the files are in pokemon/patch/studio_formulas.h.
 */

#include "pokemon/patch/studio_formulas.h"

#include <cstring>

#include "core/formula.h"
#include "core/hook.h"
#include "core/problems.h"
#include "pokemon/address.h"
#include "core/utf8.h"
#include "pokemon/patch/shiny.h"
#include "system/constant/language.h"
#include "system/native/core.h"
#include "system/native/file.h"
#include "system/native/string.h"

namespace pokemon {

namespace {

constexpr const c16* kFormulasPath = u"sdmc:/sango-studio/formula/formulas.bin";
constexpr const c16* kNaturesPath = u"sdmc:/sango-studio/nature/natures.csv";
const c8 kFormulasFile[] = "formula/formulas.bin";
const c8 kNaturesFile[] = "nature/natures.csv";

constexpr u32 kSignature = 0x4D524653; // "SFRM"
constexpr u32 kHeaderSize = 12;
constexpr u32 kGameShiny = 0xFFFFFFFFu;
constexpr u32 kShinyOutOf = 65536;
/// The number of the first natures: the natures of the game.
constexpr u32 kGameNatures = StudioFormulas::kGameNatures;

/// The effect of the natures of the game on Attack, Defense, Special Attack, Special Defense and Speed:
/// +1 raises by 10 %, -1 lowers by 10 % (the table of the game).
const s8 kGameSigns[kGameNatures][5] = {
    {0, 0, 0, 0, 0},   {1, -1, 0, 0, 0}, {1, 0, 0, 0, -1}, {1, 0, -1, 0, 0}, {1, 0, 0, -1, 0},
    {-1, 1, 0, 0, 0},  {0, 0, 0, 0, 0},  {0, 1, 0, 0, -1}, {0, 1, -1, 0, 0}, {0, 1, 0, -1, 0},
    {-1, 0, 0, 0, 1},  {0, -1, 0, 0, 1}, {0, 0, 0, 0, 0},  {0, 0, -1, 0, 1}, {0, 0, 0, -1, 1},
    {-1, 0, 1, 0, 0},  {0, -1, 1, 0, 0}, {0, 0, 1, 0, -1}, {0, 0, 0, 0, 0},  {0, 0, 1, -1, 0},
    {-1, 0, 0, 1, 0},  {0, -1, 0, 1, 0}, {0, 0, 0, 1, -1}, {0, 0, -1, 1, 0}, {0, 0, 0, 0, 0},
};

core::Formula formulas[StudioFormulas::kStatCount];
/// The change of each nature for the five statistics, in percent of the value (10 raises by 10 %, -200 gives
/// minus two times the value, which is then limited to 1).
s16 nature_change[StudioFormulas::kMaxNatures][5];
/// True when natures.csv is read: the table replaces the table of the game.
bool natures_loaded = false;

/// The names of the new natures in the language of the game.
constexpr u32 kNameSize = 20;
c16 nature_names[StudioFormulas::kMaxNatures][kNameSize];
bool nature_has_line[StudioFormulas::kMaxNatures];
sys::Language names_language = sys::Language::kNone;

/// The place of the name for the language of the game, among the eight name columns of natures.csv (English,
/// Japanese, Japanese with kanji, French, Italian, German, Spanish, Korean).
u32 NameColumn(sys::Language language) {
  switch (language) {
  case sys::Language::kJapanese: return 1;
  case sys::Language::kFrench: return 3;
  case sys::Language::kItalian: return 4;
  case sys::Language::kGerman: return 5;
  case sys::Language::kSpanish: return 6;
  case sys::Language::kKorean: return 7;
  default: return 0;
  }
}

core::Hook<u16(u32, u8, u16, u16, u16)> max_hp_hook;
core::Hook<u16(u8, u16, u16, u16, u32)> attack_hook;
core::Hook<u16(u8, u16, u16, u16, u32)> defense_hook;
core::Hook<u16(u8, u16, u16, u16, u32)> special_attack_hook;
core::Hook<u16(u8, u16, u16, u16, u32)> special_defense_hook;
core::Hook<u16(u8, u16, u16, u16, u32)> speed_hook;
core::Hook<u16(u16, u16, u32)> corrected_hook;

/// Gives the table of the natures its first values: the natures of the game, then 100 %.
void ResetNatures() {
  for (u32 nature = 0; nature < StudioFormulas::kMaxNatures; nature++) {
    for (u32 stat = 0; stat < 5; stat++) {
      nature_change[nature][stat] = nature < kGameNatures ? static_cast<s16>(10 * kGameSigns[nature][stat]) : 0;
    }
  }
  natures_loaded = false;
}

/// What the game calls a "power": 1 Attack, 2 Defense, 3 Special Attack, 4 Special Defense, 5 Speed.
u16 CorrectedHook(u16 value, u16 nature, u32 power) {
  if (!natures_loaded || power < 1 || power > 5 || nature >= StudioFormulas::kMaxNatures) {
    return corrected_hook(value, nature, power);
  }
  const s32 change = nature_change[nature][power - 1];
  const s64 result = static_cast<s64>(value) * (100 + change) / 100;
  if (result < 1) return 1;
  return static_cast<u16>(result > 0xFFFF ? 0xFFFF : result);
}

u16 Clamp(s64 value, s64 low) {
  if (value < low) return static_cast<u16>(low);
  if (value > 0xFFFF) return 0xFFFF;
  return static_cast<u16>(value);
}

u16 CalcStat(StudioFormulas::Stat stat, u8 level, u16 base, u16 iv, u16 ev, u32 nature) {
  const s32 variables[5] = {base, iv, ev, level, 0};
  const u16 raw = Clamp(formulas[stat].Evaluate(variables, 5), 1);
  return CorrectedHook(raw, static_cast<u16>(nature), static_cast<u32>(stat));
}

u16 MaxHpHook(u32 species, u8 level, u16 base, u16 iv, u16 ev) {
  if (!formulas[StudioFormulas::kHp].IsSet()) return max_hp_hook(species, level, base, iv, ev);
  const s32 variables[5] = {base, iv, ev, level, static_cast<s32>(species)};
  return Clamp(formulas[StudioFormulas::kHp].Evaluate(variables, 5), 1);
}

u16 AttackHook(u8 level, u16 base, u16 iv, u16 ev, u32 nature) {
  if (!formulas[StudioFormulas::kAttack].IsSet()) return attack_hook(level, base, iv, ev, nature);
  return CalcStat(StudioFormulas::kAttack, level, base, iv, ev, nature);
}

u16 DefenseHook(u8 level, u16 base, u16 iv, u16 ev, u32 nature) {
  if (!formulas[StudioFormulas::kDefense].IsSet()) return defense_hook(level, base, iv, ev, nature);
  return CalcStat(StudioFormulas::kDefense, level, base, iv, ev, nature);
}

u16 SpecialAttackHook(u8 level, u16 base, u16 iv, u16 ev, u32 nature) {
  if (!formulas[StudioFormulas::kSpecialAttack].IsSet()) {
    return special_attack_hook(level, base, iv, ev, nature);
  }
  return CalcStat(StudioFormulas::kSpecialAttack, level, base, iv, ev, nature);
}

u16 SpecialDefenseHook(u8 level, u16 base, u16 iv, u16 ev, u32 nature) {
  if (!formulas[StudioFormulas::kSpecialDefense].IsSet()) {
    return special_defense_hook(level, base, iv, ev, nature);
  }
  return CalcStat(StudioFormulas::kSpecialDefense, level, base, iv, ev, nature);
}

u16 SpeedHook(u8 level, u16 base, u16 iv, u16 ev, u32 nature) {
  if (!formulas[StudioFormulas::kSpeed].IsSet()) return speed_hook(level, base, iv, ev, nature);
  return CalcStat(StudioFormulas::kSpeed, level, base, iv, ev, nature);
}

u32 ReadU32(const u8* at) {
  return static_cast<u32>(at[0]) | (static_cast<u32>(at[1]) << 8) |
         (static_cast<u32>(at[2]) << 16) | (static_cast<u32>(at[3]) << 24);
}

void LoadFormulas() {
  for (u32 i = 0; i < StudioFormulas::kStatCount; i++) formulas[i].Clear();
  Shiny::SetThreshold(Shiny::kGameThreshold);
  u8 bytes[512];
  const u32 size = sys::File::ReadAll(kFormulasPath, bytes, sizeof(bytes));
  if (size == 0) return; // No file: the formulas of the game.
  if (size < kHeaderSize || ReadU32(bytes) != (kSignature) || bytes[4] != 1) {
    core::Problems::Add(u"studio: formulas.bin is not a file of this version");
    return;
  }
  const u32 count = bytes[5];
  const u32 shiny = ReadU32(bytes + 8);
  if (shiny == kGameShiny) {
    Shiny::SetThreshold(Shiny::kGameThreshold);
  } else if (shiny <= kShinyOutOf) {
    Shiny::SetThreshold(shiny);
  } else {
    core::Problems::Add(u"studio: the shiny chance of formulas.bin is too big");
  }
  u32 at = kHeaderSize;
  for (u32 i = 0; i < count; i++) {
    if (at + 2 > size) {
      core::Problems::Add(u"studio: formulas.bin is cut");
      return;
    }
    const u32 stat = bytes[at];
    const u32 length = bytes[at + 1];
    at += 2;
    if (at + length > size) {
      core::Problems::Add(u"studio: formulas.bin is cut");
      return;
    }
    if (stat >= StudioFormulas::kStatCount || !formulas[stat].Set(bytes + at, length)) {
      core::Problems::Add(u"studio: a formula of formulas.bin is not correct");
    }
    at += length;
  }
}

/// Reads a whole number with a sign at `text` (after the spaces) and moves `text` after it. The number is
/// from -1000 to 1000.
bool ReadChange(const c8** text, s32* value) {
  const c8* at = *text;
  while (*at == ' ') at++;
  bool negative = false;
  if (*at == '-') {
    negative = true;
    at++;
  } else if (*at == '+') {
    at++;
  }
  if (*at < '0' || *at > '9') return false;
  s32 out = 0;
  while (*at >= '0' && *at <= '9') {
    out = out * 10 + (*at - '0');
    if (out > 1000) return false;
    at++;
  }
  *text = at;
  *value = negative ? -out : out;
  return true;
}

/// Reads the name columns after the sixth number of a line: the name for the language of the game, or the English
/// name when it is empty. `cursor` is just after the sixth number; `end` is the end of the line.
void ReadNatureName(u32 nature, const c8* cursor, const c8* end) {
  const u32 index = nature;
  nature_has_line[index] = true;
  nature_names[index][0] = 0;
  const u32 wanted = NameColumn(names_language);
  const c8* field_start[8];
  u32 field_length[8];
  u32 count = 0;
  if (cursor < end && *cursor == ',') {
    const c8* at = cursor + 1;
    while (count < 8) {
      const c8* stop = at;
      while (stop < end && *stop != ',' && *stop != '\r') stop++;
      field_start[count] = at;
      field_length[count] = static_cast<u32>(stop - at);
      count++;
      if (stop >= end || *stop != ',') break;
      at = stop + 1;
    }
  }
  // A new nature without a name in this language takes the English name. A nature of the game without a name keeps
  // the name of the game.
  u32 column = wanted;
  if (!(wanted < count && field_length[wanted] != 0)) column = nature >= kGameNatures ? 0 : count;
  if (column < count && field_length[column] != 0) {
    core::Utf8ToUtf16(field_start[column], field_length[column], nature_names[index], kNameSize);
  }
}

void LoadNatures() {
  ResetNatures();
  names_language = sys::Core::GetInstance().GetLanguage();
  for (u32 i = 0; i < StudioFormulas::kMaxNatures; i++) {
    nature_has_line[i] = false;
    nature_names[i][0] = 0;
  }
  static c8 text[6144];
  const u32 size = sys::File::ReadAll(kNaturesPath, text, sizeof(text) - 1);
  if (size == 0) return; // No file: the natures of the game.
  text[size] = 0;
  u32 lines = 0;
  const c8* at = text;
  while (*at != 0) {
    const c8* line_end = at;
    while (*line_end != 0 && *line_end != '\n') line_end++;
    s32 values[6];
    const c8* cursor = at;
    bool ok = true;
    // Only the lines that start with a digit have a nature.
    if (*cursor >= '0' && *cursor <= '9') {
      for (u32 i = 0; i < 6 && ok; i++) {
        ok = ReadChange(&cursor, &values[i]);
        if (ok && i < 5) {
          if (*cursor != ',') {
            ok = false;
          } else {
            cursor++;
          }
        }
      }
      if (ok && values[0] >= 0 && values[0] < static_cast<s32>(StudioFormulas::kMaxNatures)) {
        for (u32 stat = 0; stat < 5; stat++) {
          nature_change[values[0]][stat] = static_cast<s16>(values[stat + 1]);
        }
        ReadNatureName(static_cast<u32>(values[0]), cursor, line_end);
        lines++;
      } else {
        core::Problems::Add(u"studio: a line of natures.csv is not correct");
      }
    }
    at = *line_end == 0 ? line_end : line_end + 1;
  }
  natures_loaded = lines != 0;
}

} // namespace

void StudioFormulas::Initialize() {
  ResetNatures();
  max_hp_hook.Install(address::kCalcMaxHp, MaxHpHook);
  attack_hook.Install(address::kCalcAttack, AttackHook);
  defense_hook.Install(address::kCalcDefense, DefenseHook);
  special_attack_hook.Install(address::kCalcSpecialAttack, SpecialAttackHook);
  special_defense_hook.Install(address::kCalcSpecialDefense, SpecialDefenseHook);
  speed_hook.Install(address::kCalcSpeed, SpeedHook);
  corrected_hook.Install(address::kCalcNatureCorrection, CorrectedHook);
}

void StudioFormulas::LoadAll() {
  LoadFormulas();
  LoadNatures();
}

bool StudioFormulas::Owns(const c8* path) {
  return std::strcmp(path, kFormulasFile) == 0 || std::strcmp(path, kNaturesFile) == 0;
}

void StudioFormulas::RefreshLanguage() {
  if (!natures_loaded) return;
  if (sys::Core::GetInstance().GetLanguage() == names_language) return;
  LoadNatures();
}

bool StudioFormulas::PatchNatureName(u32 nature, String* output) {
  if (!natures_loaded || nature >= kMaxNatures) return false;
  const u32 index = nature;
  if (!nature_has_line[index]) return false;
  if (nature_names[index][0] != 0) {
    output->Set(nature_names[index]);
    return true;
  }
  // A nature of the game without a name keeps its text.
  if (nature < kGameNatures) return false;
  // A new nature without a name: "Nature 26".
  c16 text[kNameSize] = {u'N', u'a', u't', u'u', u'r', u'e', u' '};
  u32 at = 7;
  const u32 number = nature + 1;
  if (number >= 10) text[at++] = static_cast<c16>(u'0' + number / 10);
  text[at++] = static_cast<c16>(u'0' + number % 10);
  text[at] = 0;
  output->Set(text);
  return true;
}

void StudioFormulas::Apply(const c8* path) {
  if (std::strcmp(path, kFormulasFile) == 0) LoadFormulas();
  if (std::strcmp(path, kNaturesFile) == 0) LoadNatures();
}

s32 StudioFormulas::NatureChange(u32 nature, Stat stat) {
  if (stat == kHp || nature >= kMaxNatures) return 0;
  return nature_change[nature][stat - 1];
}

} // namespace pokemon
