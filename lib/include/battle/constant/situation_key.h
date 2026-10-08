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
 * @file situation_key.h
 * @brief The keys of the situation: the values of the current moment.
 */

#pragma once
#include <types.h>

namespace battle {
/// A value of the current moment. See battle::Situation::Get().
enum class SituationKey : u8 {
  kNone = 0,
  /// A marker between nested situations. Situation::Begin() writes it.
  kScopeBoundary = 1,

  // The Pokémon
  kPokemonId = 2, ///< The Pokémon of the moment.
  kMoveUserId = 3, ///< The Pokémon that uses the current move.
  kMoveRecipientId = 4, ///< The Pokémon that receives the current move.
  kTargetCount = 5,
  kTargetId1 = 6, kTargetId2 = 7, kTargetId3 = 8,
  kTargetId4 = 9, kTargetId5 = 10, kTargetId6 = 11,

  // The selected action
  /// The kind of the selected action (fight, item, switch...).
  kActionKind = 12,
  kFieldPosition = 13,
  /// The position on the battlefield before the action.
  kOriginalFieldPosition = 14,

  // Ability
  kPreviousAbility = 15,
  kNextAbility = 16,

  // The current move
  /// A priority increase from a different source (like Quick Claw).
  kSpecialPriorityBonus = 17,
  kMoveId = 18,
  kMoveEffectId = 19, ///< The secondary effect of the move.
  /// The selected move, before a replacement (Metronome, Sleep Talk...).
  kOriginalMoveId = 20,
  /// The type that the engine checks now (a Pokémon can have several types).
  kTypeBeingChecked = 21,
  kMoveType = 22,
  /// The real type of the move when it changes (Hidden Power, Judgment...).
  kMoveTypeOverride = 23,
  kMoveSlotIndex = 24,
  kMovePriority = 25,
  kMoveUseSerial = 26, ///< A unique id for this use of the move.
  kDamageCategory = 27, ///< Physical, special or status.
  /// How the move selects its targets (one, several, the user...).
  kMoveTargetingRule = 28,
  /// The type of the user, for the same-type attack bonus (STAB).
  kMoveUserType = 29,

  // Status conditions
  kStatusId = 30,
  /// The packed data of the status condition (turns, duration...).
  kStatusData = 31,
  /// A major status condition or a volatile status condition.
  kConditionCategory = 32,

  kAmount = 33, ///< A quantity (HP that changes, HP that heal...).
  /// true when the Pokémon is in the sky or underground (Fly, Dig...).
  kIsSemiInvulnerable = 34,
  kFailureReason = 35,
  kTurnCount = 36,

  // Accuracy and evasion
  kBaseAccuracyPercent = 37,
  kBonusAccuracyPercent = 38,
  kAccuracyStage = 39,
  kEvasionStage = 40,
  kFinalAccuracyPercent = 41,

  // Multi-hit
  kMaxHitCount = 42,
  kHitCount = 43,

  kCriticalHitStage = 44,

  // Item
  kItemId = 45,
  kItemReactionEnabled = 46, ///< true when the held items can react.

  kSpeedValue = 47,

  // Damage calculation
  kMovePower = 48,
  kMovePowerMultiplier = 49,
  kDamageAmount = 50,
  kBasePowerValue = 51,
  kDefenseValue = 52,
  kMultiplier = 53,
  kSecondaryMultiplier = 54,
  /// The damage of a fixed-damage move (Seismic Toss...).
  kFixedDamageAmount = 55,
  kTypeEffectiveness = 56,

  kWeather = 57,
  /// Why the Pokémon did not faint (Sturdy, Focus Sash, Endure...).
  kSurvivalReason = 58,
  kSwapTargetId = 59,

  kVisualEffectId = 60,
  kVisualSwapCount = 61,
  kMoveExecutionMode = 62, ///< The move can run, must fail, or must stop.
  /// The address of extra data of the moment (for example, the values of a
  /// message).
  kExtraDataPointer = 63,

  // Result flags
  kNoEffectFlag = 64,
  kFailedFlag = 65,
  kMissedFlag = 66,
  /// Shows the failure message also when it is not a real failure.
  kShowFailureMessageFlag = 67,
  kStabFlag = 68, ///< The same-type attack bonus (STAB) applies.
  kCriticalHitFlag = 69,
  kSubstituteFlag = 70, ///< A substitute blocked the move.
  kOvercoatGuardFlag = 71, ///< Overcoat blocks the weather or the powder.
  kSheerForceFlag = 72, ///< Sheer Force removes the secondary effect.
  kItemSwapFlag = 73, ///< An item exchange (Trick, Switcheroo).
  kResetStatsFlag = 74, ///< Resets the stat stages of this Pokémon.
  /// Resets the stat stages of all the Pokémon (like Haze).
  kResetAllStatsFlag = 75,
  kDelayedAttackFlag = 76,
  kMagicCoatFlag = 77, ///< Magic Coat sent the move back.
  kShowMessageFlag = 78,
  kFixedValueFlag = 79, ///< Goes with kFixedDamageAmount.
  kBerryFlag = 80,
  kCheckResultFlag = 81, ///< A shared yes or no result for many checks.
  kBurnPreventedFlag = 82,

  kFieldSide = 83,
  kTeamEffectId = 84,
  kAuraBreakFlag = 85,
  kAuraPendingFlag = 86, ///< An aura effect waits to apply.
  kEffectNumber = 87, ///< The number of a visual or mechanical effect.

  kCount = 88,
};
}