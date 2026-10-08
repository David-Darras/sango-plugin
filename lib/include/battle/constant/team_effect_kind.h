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
 * @file team_effect_kind.h
 * @brief The effects on one side of the battlefield.
 */

#pragma once
#include <types.h>

namespace battle {
/// An effect on one side of the battlefield: screens, hazards...
enum class TeamEffectKind : u8 {
  kReflect = 0, ///< Halves the physical damage.
  kLightScreen, ///< Halves the special damage.
  kSafeguard, ///< Protects from the major status conditions.
  kMist, ///< Protects from the stat decreases.
  kTailwind, ///< Doubles the Speed.
  kLuckyChant, ///< Protects from the critical hits.
  kSpikes, ///< Damages the Pokémon that enter (3 layers at most).
  kToxicSpikes, ///< Poisons the Pokémon that enter (2 layers at most).
  /// Damages the Pokémon that enter. The damage depends on the type.
  kStealthRock,
  kWideGuard, ///< Blocks the moves that hit several Pokémon.
  kQuickGuard, ///< Blocks the priority moves.
  /// Pledge combination: doubles the chance of the secondary effects.
  kRainbow,
  /// Pledge combination: damages the Pokémon that are not Fire type at each
  /// turn.
  kSeaOfFire,
  kSwamp, ///< Pledge combination: divides the Speed by 4.
  kStickyWeb,
  kMatBlock, ///< Blocks the damaging moves (first turn only).
  kCraftyShield, ///< Blocks the status moves.

  kCount,
};
}
