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
 * @file turn_marker.h
 * @brief The markers of a Pokémon that the engine clears at the end of each turn.
 */

#pragma once
#include <types.h>

namespace battle {
/// A marker that the engine clears at the end of each turn.
enum class TurnMarker : u8 {
  kActionStarted,
  kActionFinished,
  kTookDamage,
  kMoveProcessingDone,
  kFlinched,
  /// The Pokémon charges Focus Punch. A hit makes it flinch.
  kBracingForFocusPunch,
  kHitWhileBracingForFocusPunch,
  kProtectSucceeded,
  kItemConsumedAndGone,
  kItemUnusable,
  kComboMoveReady, ///< The Pokémon can make a Pledge combination.
  /// The Pokémon must come back from the sky or from underground.
  kNeedsToExitHiddenState,
  kMovedOrSwitchedThisTurn,
  kTurnCheckStatusPassed, ///< The end-of-turn status check is done.
  /// The accuracy of the Pokémon is higher (like Micle Berry).
  kAccuracyBoostedByBerry,
  kUsingFling,
  /// The protection blocks only the damaging moves.
  kProtectSucceededDamageMovesOnly,
  kItemConsumptionConfirmed,

  kCount,
};
}
