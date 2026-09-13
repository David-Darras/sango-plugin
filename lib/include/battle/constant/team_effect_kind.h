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

#pragma once
#include <types.h>

namespace battle {
enum class TeamEffectKind : u8 {
  kReflect = 0, ///< Halves incoming physical damage
  kLightScreen, ///< Halves incoming special damage
  kSafeguard, ///< Immune to major status conditions
  kMist, ///< Immune to stat-lowering effects
  kTailwind, ///< Doubles Speed
  kLuckyChant, ///< Immune to critical hits
  kSpikes, ///< Damages a switched-in Pokémon (3 stackable layers)
  kToxicSpikes, ///< Poisons a switched-in Pokémon (2 stackable layers)
  kStealthRock, ///< Damages a switched-in Pokémon, scaled by type effectiveness
  kWideGuard, ///< Blocks moves that hit the whole field or everyone but the user
  kQuickGuard, ///< Blocks priority moves
  kRainbow, ///< Elemental-combo effect: doubles secondary-effect chance
  kSeaOfFire, ///< Elemental-combo effect: damages non-Fire types each turn
  kSwamp, ///< Elemental-combo effect: quarters Speed
  kStickyWeb,
  kMatBlock, ///< Blocks damaging moves (first-turn-only Protect variant)
  kCraftyShield, ///< Blocks status moves

  kCount,
};
}
