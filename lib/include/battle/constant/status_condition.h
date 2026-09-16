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
enum class StatusCondition : u8 {
  kNone = 0,
  kParalysis = 1,
  kSleep = 2,
  kFreeze = 3,
  kBurn = 4,
  kPoison = 5,
  kConfusion = 6,
  kInfatuation = 7,
  kBound = 8, ///< Wrap/Fire Spin/Bind-style multi-turn trapping damage
  kNightmare = 9,
  kCurse = 10,
  kTaunt = 11,
  kTorment = 12,
  kDisable = 13,
  kYawn = 14, ///< Falls asleep next turn
  kHealBlock = 15,
  kAbilitySuppressed = 16, ///< Gastro Acid
  kIdentified = 17,
  ///< Foresight/Odor Sleuth - negates evasion and Ghost-type immunity
  kLeechSeed = 18,
  kEmbargo = 19, ///< Can't use items
  kPerishSong = 20,
  kIngrain = 21,
  kEscapePrevented = 22, ///< Mean Look/Block/Spider Web
  kEncore = 23,
  kRoosting = 24, ///< Temporarily loses the Flying type
  kMoveLockedNoSelect = 25,
  ///< Can only repeat the last move used (Outrage, Rollout...), can't even
  ///< open the menu
  kLockedToChargingMove = 26, ///< Locked into an in-progress charging move
  kChoiceLockedToFirstMove = 27, ///< Choice Band/Specs/Scarf-style lock
  kAlwaysHits = 28, ///< This Pokémon's next attack can't miss (Lock-On/Mind Reader)
  kMarkedByLockOn = 29,
  ///< Specifically targeted by another Pokémon's Lock-On/Mind Reader
  kLevitating = 30, ///< Magnet Rise
  kLevitationBlocked = 31,
  ///< Can't gain a floating/Ground-immune state (Ingrain, Smack Down...)
  kTelekinesis = 32,
  kFreeFall = 33, ///< Sky Drop's carry state
  kAccuracyBoosted = 34, ///< Micle Berry-style temporary accuracy boost
  kAquaRing = 35,
  kForcedMoveType = 36,
  ///< Electrify - forces the next move used to become Electric-type
  kPowderCoated = 37,
  ///< Explodes for 25% max HP and cancels the move if a Fire-type move is used
  kCount = 38,
};

} // namespace battle

using battle::StatusCondition;
