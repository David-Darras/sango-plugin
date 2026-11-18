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
 * @file studio_trainers.h
 * @brief What Sango Studio adds to the teams of the trainers.
 *
 * The data, the teams and the classes of the trainers are the files of the
 * game archives that the archive layer replaces (see core::Archive). The
 * file `sdmc:/sango-studio/trainer/extra/<number>.bin` has what the files of
 * the game cannot give: the weather, the terrain and the effects on the
 * field at the start of the battle, and for each Pokémon the nature, the
 * shiny state, the exact ability, the EVs, the IVs of each stat and a
 * nickname. The plugin reads it when the battle with the trainer starts.
 *
 * The file has the text `STRX`, the version 1, the number of Pokémon, the
 * weather (255: the one of the place), the terrain, the effects on the field
 * (one bit each: Trick Room, Gravity, Magic Room, Wonder Room) and 56 bytes
 * for each Pokémon. The writer is `module/core/src/trainer.rs` in Sango
 * Studio.
 */

#pragma once

#include "common.h"
#include "battle/constant/trainer.h"
#include "battle/native/reaction_table.h"

namespace battle {
struct Config;

/// Applies the extras of Sango Studio to the team of a trainer.
class StudioTrainers {
public:
  /// Reads the extras of the trainer and applies them to the team of the
  /// opponent. Call it after the team of the game is made. Does nothing when
  /// the trainer has no file.
  static void Apply(Config& config, TrainerId trainer);

  /// The reactions of the start of the battle (the terrain and the effects on
  /// the field). The listener of the first Pokémon that enters takes them.
  static const ReactionTable* StartReactions(u32* count);

  /// True while the effects of the start of the battle wait for the first
  /// Pokémon that enters, and the listener was not made yet.
  static bool WantsStartListener();
  /// Tells that the listener of the start was made.
  static void StartListenerMade();
};

} // namespace battle
