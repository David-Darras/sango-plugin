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
 * @file persistent_marker.h
 * @brief The markers of a Pokémon that stay while it is in battle.
 */

#pragma once
#include <types.h>

namespace battle {
/// A marker that stays from the moment a Pokémon enters the battle until it
/// leaves.
enum class PersistentMarker : u8 {
  kActedThisTurn,
  kCantSwitchOrFlee,
  kCharging, ///< The Pokémon charges a two-turn move.
  kFlying, ///< The Pokémon is in the sky (Fly).
  kDiving, ///< The Pokémon is under the water (Dive).
  kDigging, ///< The Pokémon is underground (Dig).
  kShadowForce, ///< The Pokémon is hidden (Shadow Force).
  kCurledUp, ///< The Pokémon used Defense Curl.
  kMinimized,
  /// The critical hit stage of the Pokémon is higher (Focus Energy).
  kFocusingEnergy,
  kPowerTrickActive,
  kMicleBerryBoostReady,
  kCantActFromRecoil, ///< The Pokémon must recharge (like Hyper Beam).
  /// Flash Fire: the Pokémon is immune to Fire, and its Fire moves are 1.5
  /// times stronger.
  kFlashFireActivated,
  kBatonTouchPending,
  kLostHeldItem,
  kElectricTerrainGuard, ///< Electric Terrain protects the Pokémon from sleep.
  /// Misty Terrain protects the Pokémon from the status conditions.
  kMistyTerrainGuard,

  kCount,
};
}
