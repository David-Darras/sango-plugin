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
 * @file studio_formulas.h
 * @brief The formulas of the statistics, the natures and the shiny chance of Sango Studio.
 *
 * In the pages "Formula" and "Nature" of Sango Studio, the user changes how the game calculates the
 * statistics of a Pokémon, what each nature does, and the chance of a shiny Pokémon. Sango Studio
 * writes two files in `sdmc:/sango-studio`:
 *
 * - `formula/formulas.bin`: the formulas of the six statistics and the shiny chance.
 * - `nature/natures.csv`: the effect of each nature on the five statistics, in percent.
 *
 * The layout of `formulas.bin` (all the numbers are little-endian):
 *
 * | Offset | Size | Content |
 * |---|---|---|
 * | 0 | 4 | The text `SFRM`. |
 * | 4 | 1 | The version of the layout: 1. |
 * | 5 | 1 | The number of formulas. |
 * | 6 | 2 | Not used. |
 * | 8 | 4 | The shiny chance, in 65536ths (0 to 65536), or 0xFFFFFFFF for the chance of the game. |
 * | 12 | ... | For each formula: the number of the statistic (1 byte), the size of the program (1 byte), then the program (see core/formula.h). |
 *
 * The statistics are numbered 0 to 5: HP, Attack, Defense, Special Attack, Special Defense, Speed. The
 * program of a statistic reads these variables:
 *
 * | Variable | Content |
 * |---|---|
 * | 0 | The base value of the species. |
 * | 1 | The IV. |
 * | 2 | The EV. |
 * | 3 | The level. |
 * | 4 | The species (for the HP only: the other statistics get 0). |
 *
 * The program gives the value before the nature. The nature then changes it by a percent. The game
 * does `value * 110 / 100` (raise by 10 %) or `value * 90 / 100` (lower by 10 %): this feature does
 * `value * (100 + change) / 100`, where `change` is any whole number of percent. A change of -200
 * gives minus two times the value, so the statistic is 1 (a statistic is never under 1).
 *
 * The file `natures.csv` has one line for each nature: `number,attack,defense,special attack,special
 * defense,speed,name,ja,ja_kanji,fr,it,de,es,ko` (the changes are whole numbers of percent from -1000 to
 * 1000, with a sign if they are negative; 0 changes nothing, 10 raises by 10 %, -10 lowers by 10 %). The
 * eight names are the name of the nature in English and in the other languages; an empty name is the
 * English name for a new nature, and the name of the game for a nature 0 to 24. The name columns are
 * optional. The game shows these names in the language of the game. The lines
 * that start with something else than a digit are ignored. The numbers 0 to 24 are the natures of
 * the game. The numbers 25 to 31 are new natures: they work for the statistics only (the game does not
 * know them for the flavors and for the texts).
 *
 * A formula or a nature that the files do not have keeps the calculation of the game. core::StudioFiles
 * calls this feature, so it works in the overlay and in every product that calls
 * core::StudioFiles::Load(). The reload counter of `reload.bin` makes it read the files again.
 */

#pragma once

#include "common.h"

struct String;

namespace pokemon {

/// Changes the calculation of the statistics, the effect of the natures and the shiny chance.
class StudioFormulas {
public:
  /// The statistics, in the order of the files.
  enum Stat : u8 { kHp, kAttack, kDefense, kSpecialAttack, kSpecialDefense, kSpeed, kStatCount };
  /// The most natures that a table can have.
  static constexpr u32 kMaxNatures = 32;
  /// The number of natures of the game: the first new nature has this number.
  static constexpr u32 kGameNatures = 25;

  /// Installs the hooks of the calculations. Call it once.
  static void Initialize();

  /// Reads `formula/formulas.bin` and `nature/natures.csv`.
  static void LoadAll();

  /// Reads the file with this path (relative to the folder of the studio) when it is one of these files.
  static void Apply(const c8* path);

  /// Returns true when the path is one of the files of this feature.
  static bool Owns(const c8* path);

  /// Reads the names of the new natures again when the language of the game changed. core::StudioFiles calls it.
  static void RefreshLanguage();

  /// Gives the name that the file has for a nature, in the language of the game. Returns false when the
  /// file has no name for it: the caller keeps the text of the game.
  static bool PatchNatureName(u32 nature, String* output);

  /// Formats the Trainer Memo string (file 28) for the given nature (0 to 24).
  /// Returns false when the nature was not renamed: the game keeps its vanilla sentence.
  static bool PatchNatureTrainerMemo(u32 nature, String* output);

  /// The change of a statistic that a nature gives, in percent (0 when the nature does not change it).
  static s32 NatureChange(u32 nature, Stat stat);
};

} // namespace pokemon
