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
 *   evolutions of the species, and the new species ([new Sangoat, Skiddo]),
 * - `story.txt`: the scripts of the characters and of the events (talk,
 *   choices, conditions, battles, warps, gifts...),
 * - `characters.txt`: the characters and the trainers to add to the maps,
 * - `rules.txt`: the rules of the battles, the shiny rate and the starters,
 * - `shops.txt`: the items and the Pokémon that the shops sell,
 * - `items.txt`: the price, the power of Fling, the HP and the EVs of the
 *   items,
 * - `moves.txt`: the type, the category, the power, the accuracy, the PP and
 *   the priority of the moves.
 *
 * The files use the names of the library, in English: "Pikachu",
 * "Thunder Shock", "Route 101". The case, the spaces and the signs do not
 * matter: "THUNDER_SHOCK" and "thundershock" are also good. A number is also
 * good. A mistake goes to the screen of the problems at the start.
 *
 * The Sango Maker product (maker/) calls DataFiles::Load(). A product in C++
 * can call it too, in SetUpHack().
 *
 * Reload() reads the files again while the game runs (the Sango Maker page
 * of the menu). With `is_live`, Update() reads them again when they change.
 * Load() adds the page to the menu and Update() to the frames: a product
 * that does not call Load() does not contain the code of the files.
 *
 * @see docs/tutorials/18-make-a-rom-hack-without-code.md
 */

#pragma once

#include "common.h"
#include "core/maker_reader.h"

namespace core {

/// Reads the data files of the ROM hack.
class DataFiles {
  MAKE_SINGLETON(DataFiles)

public:
  /// The number of problems that GetProblem() keeps.
  static constexpr u32 kMaxProblems = 16;
  /// The length of a problem, with the end of the text.
  static constexpr u32 kMaxProblemLength = 64;

  bool is_live = false; ///< Reads the files again when they change.
  bool trace_story = false; ///< Writes each command of story.txt to the log.

  /// Reads the files of the list above. Call it in SetUpHack().
  static void Load();

  /**
   * @brief Removes the data of the files from the game, then reads the files
   *        again.
   *
   * A change of the stats, of the types, of the abilities, of the TMs or of
   * the catch rate stays until the next start when its line goes away. The
   * map loads again at the next step of the player, with the new characters
   * and wild Pokémon.
   *
   * @return false when a script of story.txt runs: Update() reads the files
   *         after the script.
   */
  static bool Reload();

  /// Reads the files again after a Reload() that waits, or when a file
  /// changes (`is_live`). Load() makes the plugin call it at each frame.
  static void Update();

  /// Returns the number of problems of the last read of the files.
  static u32 GetProblemCount();
  /// Returns a problem of the last read of the files (UTF-8).
  static const c8* GetProblem(u32 index);
  /// Returns the number of Reload() since the start.
  static u32 GetReloadCount();

  static bool LoadEncounters(const c16* path);
  static bool LoadTrainers(const c16* path);
  static bool LoadSpecies(const c16* path);
  /// Reads the scripts of story.txt, and registers them (ids 33000 to
  /// 33999).
  static bool LoadStory(const c16* path);
  /// Reads the characters of characters.txt, and adds them to the maps with
  /// overworld::MapCharacter::Add() (128 characters at most).
  static bool LoadCharacters(const c16* path);
};

} // namespace core
