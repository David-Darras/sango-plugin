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
 * @file battle_result.h
 * @brief How a battle of a script ends.
 */

#pragma once

#include <types.h>

namespace script {

/// How a battle ends. script::Context::TrainerBattle() and WildBattle()
/// return it.
enum class BattleResult : u8 {
  kLost = 0, ///< The player lost, or the battle was a draw.
  kWon = 1, ///< The player won.
  kRan = 2, ///< The player ran away (wild battle).
  kFoeRan = 3, ///< The wild Pokémon ran away.
  kCaught = 4, ///< The player caught the wild Pokémon.
  kNotStarted = 0xFF, ///< The battle did not start (see the log).
};

} // namespace script
