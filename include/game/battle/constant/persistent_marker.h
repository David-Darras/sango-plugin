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
// Held from the moment a Pokémon enters battle until it leaves.
enum class PersistentMarker : u8 {
  kActedThisTurn,
  kCantSwitchOrFlee,
  kCharging, ///< Mid-charge for a two-turn move
  kFlying, ///< Vanished via Fly
  kDiving, ///< Vanished via Dive
  kDigging, ///< Vanished via Dig
  kShadowForce, ///< Vanished via Shadow Force
  kCurledUp, ///< Defense Curl-style curled state
  kMinimized,
  kFocusingEnergy, ///< Mid-buildup for a Focus Energy-style critical-hit setup
  kPowerTrickActive,
  kMicleBerryBoostReady,
  kCantActFromRecoil, ///< Immobilized as a recharge cost (Hyper Beam-style)
  kFlashFireActivated, ///< Immune to Fire and its own Fire-type moves are boosted 1.5x
  kBatonTouchPending,
  kLostHeldItem,
  kElectricTerrainGuard, ///< Protected against sleep by Electric Terrain
  kMistyTerrainGuard, ///< Protected against status by Misty Terrain

  kCount,
};
}
