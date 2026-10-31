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
 * @file data_files.h
 * @brief A ROM hack without code: text files that change the game.
 *
 * DataFiles::Load() reads these files of the data folder (see
 * core/data_folder.h), when they exist:
 * - `encounters.txt`: the wild Pokémon of the maps,
 * - `trainers.txt`: the teams and the data of the trainers,
 * - `species.txt`: the stats, the types, the TMs, the learnsets and the
 *   evolutions of the species.
 *
 * The files use the names of the library, in English: "Pikachu",
 * "Thunder Shock", "Route 101". The case, the spaces and the signs do not
 * matter: "THUNDER_SHOCK" and "thundershock" are also good. A number is also
 * good. A mistake goes to the screen of the problems at the start.
 *
 * The Sango Maker product (maker/) calls DataFiles::Load(). A product in C++
 * can call it too, in SetUpHack().
 *
 * @see docs/tutorials/18-make-a-rom-hack-without-code.md
 */

#pragma once

#include "common.h"

namespace core {

/// The kinds of names of the data files.
enum class NameKind : u8 {
  kSpecies,
  kMove,
  kItem,
  kAbility,
  kNature,
  kType,
  kForm,
  kTrainer,
  kModel,
  kEvolutionMethod,
  kEncounterMethod,
  kFormat,
  kMap,
};

/// Finds the values of the names of the data files.
class DataNames {
public:
  /**
   * @brief Finds the value of a name ("Thunder Shock") or of a number ("84").
   * @param text The name. It does not need a 0 at the end.
   * @param length The number of bytes of the name.
   * @param value The value.
   * @return false when the name does not exist.
   */
  static bool Find(NameKind kind, const c8* text, u32 length, u32* value);
};

/// Reads the data files of the ROM hack.
class DataFiles {
public:
  /// Reads encounters.txt, trainers.txt and species.txt. Call it in
  /// SetUpHack().
  static void Load();

  static bool LoadEncounters(const c16* path);
  static bool LoadTrainers(const c16* path);
  static bool LoadSpecies(const c16* path);
};

} // namespace core
