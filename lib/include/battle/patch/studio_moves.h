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
 * @file studio_moves.h
 * @brief The new moves and the changed moves of Sango Studio.
 *
 * In the page "Move" of Sango Studio, the user changes the data of the moves of
 * the game and makes new moves. Sango Studio writes these files in
 * `sdmc:/sango-studio`:
 *
 * - `move/data/[id].bin`: the data of a move of the game (36 bytes, the layout
 *   of pokemon::MoveData). The plugin gives this data to the game each time it
 *   loads the move.
 * - `move/custom/[id].bin`: a new move, with an id from 900 to 931.
 *
 * The layout of the file of a new move (all the numbers are little-endian):
 *
 * | Offset | Size | Content |
 * |---|---|---|
 * | 0 | 4 | The text `SMVR`. |
 * | 4 | 1 | The version of the layout: 1. |
 * | 5 | 2 | The id of the move. |
 * | 7 | 1 | The number of rules. |
 * | 8 | ... | For each of the eight languages: the name, then the description (see battle/patch/studio_abilities.h for the texts). |
 * | ... | 36 | The data of the move (pokemon::MoveData). |
 * | ... | ... | The rules (see battle/patch/studio_rules.h). |
 *
 * The plugin registers each new move with battle::GameExtension::AddMove().
 * The reactions of the move run the rules. core::StudioFiles calls this
 * feature, so it works in the overlay and in every product that calls
 * core::StudioFiles::Load(). ORAS only.
 *
 * @see docs/tutorials/04-add-a-move.md
 */

#pragma once

#include "common.h"

namespace battle {

/// Reads the move files of Sango Studio and runs their rules in battle.
class StudioMoves {
public:
  /// The first id and the last id of a move of the studio.
  static constexpr u32 kFirstId = 900;
  static constexpr u32 kLastId = 931;
  /// The most new moves that the studio can add at the same time.
  static constexpr u32 kMaxMoves = 32;
  /// The most changed moves of the game.
  static constexpr u32 kMaxChanges = 96;
  static constexpr u32 kNameSize = 28;
  static constexpr u32 kDescriptionSize = 96;

  /// Reads the files `move/custom/900.bin` to `move/custom/931.bin`. Call it
  /// one time after the SD card is ready.
  static void LoadAll();

  /// Reads one file again. `path` is the path in reload.bin, for example
  /// "move/custom/900.bin" or "move/data/85.bin".
  static void Apply(const c8* path);

  /// Takes the texts of the language of the game again when the player changes
  /// the language. Call it often: it is cheap.
  static void RefreshLanguage();
};

} // namespace battle
