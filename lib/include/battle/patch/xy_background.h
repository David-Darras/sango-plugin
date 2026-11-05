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
 * @file xy_background.h
 * @brief Shows a battle background of Pokémon X and Y in a test battle.
 */

#pragma once

#include "common.h"

namespace battle {

/**
 * @brief Gives a battle pieces of a background of X and Y (ORAS only).
 *
 * The files are in sdmc:/sango/battle_background (see
 * tools/export_xy_battle_backgrounds.py). A background has several pieces
 * (the ground, the plate, the far ground, the background behind), and each
 * piece is one file of the archive of the backgrounds. This feature makes the
 * game read new files in place of files of ORAS, for one battle. The page of
 * the static encounters starts the battle.
 */
class XyBackground {
  MAKE_SINGLETON(XyBackground)

public:
  /// The number of files in the archive of ORAS: the first new file.
  static constexpr u32 kFirstNewFile = 158;
  /// The kinds of pieces that a battle can choose.
  enum Kind : u32 { kGround, kFarGround, kBack, kPlate, kKindCount };
  /// The name of the kind in the names of the lists ("ground"...).
  static const c8* GetKindName(u32 kind);

  /// The choice of each kind. 0: the piece that the game gives. The other
  /// numbers are the entries of the list of the kind: first the pieces of
  /// ORAS, then the pieces of X and Y (see choices_<kind>.bin).
  u32 choices[kKindCount] = {};

  /// Makes the game read the chosen pieces in place of the pieces that it
  /// would read, for one battle. A kind with the choice 0 does not change.
  static void Apply();
  /// Gives the file of ORAS back when the battle is over. Call it each frame.
  static void Update();

private:
  static constexpr u32 kMaxApplied = 256;

  bool is_applied_ = false;
  bool is_battle_seen_ = false;
  u32 applied_files_[kMaxApplied] = {};
  u32 applied_count_ = 0;
  static void RemoveApplied();
};
} // namespace battle
