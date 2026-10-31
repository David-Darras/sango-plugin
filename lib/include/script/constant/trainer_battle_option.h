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
 * @file trainer_battle_option.h
 * @brief The options of a trainer battle that a script starts.
 */

#pragma once

#include <types.h>

#include "core/bitmask.h"

namespace script {

/// The options of script::Context::TrainerBattle(). Use | to give more than
/// one option.
enum class TrainerBattleOption : u32 {
  kNone = 0,
  kSkyBattle = 1 << 0, ///< A Sky Battle.
  kInverse = 1 << 1, ///< An Inverse Battle: the type chart is reversed.
  /// The player cannot lose: after a loss, the script continues.
  kCannotLose = 1 << 2,
  kMoreMoney = 1 << 3, ///< The player gets 1.5 times the prize money.
  kLevelUp5 = 1 << 4, ///< The Pokémon of the trainer get 5 levels more.
  kLevelUp10 = 1 << 5, ///< The Pokémon of the trainer get 10 levels more.
  kLevelUp20 = 1 << 6, ///< The Pokémon of the trainer get 20 levels more.
  kLevelDown10 = 1 << 7, ///< The Pokémon of the trainer get 10 levels less.
};
ENABLE_BITMASK_OPERATORS(TrainerBattleOption)

} // namespace script
