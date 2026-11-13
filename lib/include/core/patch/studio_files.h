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
 * @file studio_files.h
 * @brief Loads the files that Sango Studio writes on the SD card.
 *
 * Sango Studio is the desktop editor of the game data. It writes each file
 * that the user changes in sdmc:/sango-studio, and it lists the file in
 * reload.bin. The plugin loads the files after the start, and loads them
 * again while the game runs when reload.bin changes.
 *
 * reload.bin has the signature "SREL" (4 bytes), a counter (4 bytes), the
 * number of paths (4 bytes), then one path of 64 bytes for each written
 * file, filled with zeros. The studio raises the counter after each write.
 *
 * The files:
 * - pokemon/data/[species]_[form].bin: the data of one species or of one
 *   form (80 bytes, the layout of pokemon::SpeciesData). The plugin writes
 *   the data over the entry of the table of the species.
 * - pokemon/pokedex_data.bin: the Pokédex data file (the shapes and the
 *   chain of the forms). The game reads it in place of the file of the
 *   archive (see core::Archive::SetPokedexDataFile).
 * - ability/[id].bin: a new ability that the user made in the studio (an id
 *   from 192 to 255). The plugin reads all these files at the first load, then
 *   each file that reload.bin lists. See battle::StudioAbilities.
 * - type/types.csv, type/icon/[id].bclim and type/chart.bin: the new types,
 *   the icons and the type chart that the user made. See battle::StudioTypes.
 * - move/data/[id].bin and move/custom/[id].bin: the changed moves of the game
 *   and the new moves. See battle::StudioMoves.
 * - item/data/[id].bin, item/custom/[id].bin and item/custom/[id].bclim: the
 *   changed items and the new items. See battle::StudioItems.
 */

#pragma once

#include "common.h"

namespace core {

/// Loads the files of Sango Studio from the SD card, and loads them again
/// when the studio changes them while the game runs.
class StudioFiles {
  MAKE_SINGLETON(StudioFiles)

public:
  /// Watches the folder of Sango Studio. Call it once at the start.
  /// Alpha Sapphire only: the other games do nothing.
  static void Load();

private:
  /// The size of one path in reload.bin.
  static constexpr u32 kPathSize = 64;
  /// The most paths in reload.bin.
  static constexpr u32 kMaxPaths = 256;
  /// The frames between two looks at reload.bin (about two looks a second).
  static constexpr u32 kPollFrames = 30;

  /// Looks at reload.bin, and loads the files when the counter changed.
  static void Update();
  /// Loads all the files that reload.bin lists.
  static void LoadAll();
  /// Loads one file from its path in reload.bin.
  static void Apply(const c8* path);
  /// Loads the data of one species or form
  /// ("pokemon/data/[species]_[form].bin").
  static void ApplySpeciesData(const c8* path);
  /// Tells core::Archive the size of the Pokédex data file of the studio.
  static void RefreshPokedexData();

  /// True after the first load.
  bool loaded_ = false;
  /// The counter of reload.bin at the last load.
  u32 counter_ = 0;
};

} // namespace core
