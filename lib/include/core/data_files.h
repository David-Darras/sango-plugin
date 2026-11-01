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
 *   evolutions of the species,
 * - `story.txt`: the scripts of the characters and of the events (talk,
 *   choices, conditions, battles, warps, gifts...),
 * - `characters.txt`: the characters and the trainers to add to the maps.
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
#include "core/maker_reader.h"

namespace core {

/// Reads the data files of the ROM hack.
class DataFiles {
public:
  /// Reads encounters.txt, trainers.txt, species.txt, story.txt and
  /// characters.txt. Call it in SetUpHack().
  static void Load();

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
