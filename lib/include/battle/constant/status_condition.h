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
 * @file status_condition.h
 * @brief The status conditions.
 */

#pragma once

#include <types.h>

namespace battle {
/// A status condition: the major ones (burn, freeze...) and the volatile ones (confusion, taunt...).
enum class StatusCondition : u8 {
  kNone = 0,
  kParalysis = 1,
  kSleep = 2,
  kFreeze = 3,
  kBurn = 4,
  kPoison = 5,
  kConfusion = 6,
  kInfatuation = 7,
  kBound = 8, ///< Damage during several turns (Wrap, Fire Spin, Bind...).
  kNightmare = 9,
  kCurse = 10,
  kTaunt = 11,
  kTorment = 12,
  kDisable = 13,
  kYawn = 14, ///< The Pokémon falls asleep at the next turn.
  kHealBlock = 15,
  kAbilitySuppressed = 16, ///< The ability does not work (Gastro Acid).
  kIdentified = 17, ///< Foresight, Odor Sleuth: no evasion, no Ghost immunity.
  kLeechSeed = 18,
  kEmbargo = 19, ///< The Pokémon cannot use items.
  kPerishSong = 20,
  kIngrain = 21,
  /// The Pokémon cannot leave (Mean Look, Block, Spider Web).
  kEscapePrevented = 22,
  kEncore = 23,
  kRoosting = 24, ///< The Pokémon loses the Flying type for this turn.
  /// The Pokémon must use its last move again (Outrage, Rollout...). The menu
  /// does not open.
  kMoveLockedNoSelect = 25,
  kLockedToChargingMove = 26, ///< The Pokémon must continue its charge move.
  /// The Pokémon must use its first move (Choice Band, Specs, Scarf).
  kChoiceLockedToFirstMove = 27,
  /// The next attack of this Pokémon always hits (Lock-On, Mind Reader).
  kAlwaysHits = 28,
  /// A different Pokémon used Lock-On or Mind Reader on this Pokémon.
  kMarkedByLockOn = 29,
  kLevitating = 30, ///< Magnet Rise.
  /// The Pokémon cannot float (Ingrain, Smack Down...).
  kLevitationBlocked = 31,
  kTelekinesis = 32,
  kFreeFall = 33, ///< Sky Drop carries the Pokémon.
  kAccuracyBoosted = 34, ///< A temporary accuracy increase (like Micle Berry).
  kAquaRing = 35,
  /// Electrify: the next move of the Pokémon becomes an Electric move.
  kForcedMoveType = 36,
  /// Powder: a Fire move explodes, gives 25% damage and fails.
  kPowderCoated = 37,
  kCount = 38,
};

} // namespace battle

using battle::StatusCondition;
