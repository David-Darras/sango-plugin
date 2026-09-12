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

enum class StatusId : u8 {
  kNone = 0,
  kParalysis,
  kSleep,
  kFreeze,
  kBurn,
  kPoison,
  kConfusion,
  kInfatuation,
  kBound, ///< Wrap/Fire Spin/Bind-style multi-turn trapping damage
  kNightmare,
  kCurse,
  kTaunt,
  kTorment,
  kDisable,
  kYawn, ///< Falls asleep next turn
  kHealBlock,
  kAbilitySuppressed, ///< Gastro Acid
  kIdentified,
  ///< Foresight/Odor Sleuth - negates evasion and Ghost-type immunity
  kLeechSeed,
  kEmbargo, ///< Can't use items
  kPerishSong,
  kIngrain,
  kEscapePrevented, ///< Mean Look/Block/Spider Web
  kEncore,
  kRoosting, ///< Temporarily loses the Flying type
  kChoiceLockedToFirstMove, ///< Choice Band/Specs/Scarf-style lock
  kLockedToChargingMove, ///< Locked into an in-progress charging move
  kMoveLockedNoSelect,
  ///< Can only repeat the last move used, can't even open the menu
  kAlwaysHits, ///< This Pokémon's next attack can't miss (Lock-On/Mind Reader)
  kMarkedByLockOn,
  ///< Specifically targeted by another Pokémon's Lock-On/Mind Reader
  kLevitating, ///< Magnet Rise
  kLevitationBlocked,
  ///< Can't gain a floating/Ground-immune state (Ingrain, Smack Down...)
  kTelekinesis,
  kFreeFall, ///< Sky Drop's carry state
  kAccuracyBoosted, ///< Micle Berry-style temporary accuracy boost
  kAquaRing,
  kForcedMoveType,
  ///< Electrify - forces the next move used to become Electric-type
  kPowderCoated, ///< Explodes for 25% max HP and cancels the move if a Fire-type move is used
  kCount,
};