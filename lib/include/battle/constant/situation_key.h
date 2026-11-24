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

  // The Pokémon.
  kPokemonId = 2, ///< The Pokémon of the moment.
  kMoveUserId = 3, ///< The Pokémon that uses the current move.
  kMoveRecipientId = 4, ///< The Pokémon that receives the current move.
  kTargetCount = 5,
  kTargetId1 = 6, kTargetId2 = 7, kTargetId3 = 8,
  kTargetId4 = 9, kTargetId5 = 10, kTargetId6 = 11,

  // The selected action.
  /// The kind of the selected action (fight, item, switch...).
  kActionKind = 12,
  kFieldPosition = 13,
  /// The position on the battlefield before the action.
  kOriginalFieldPosition = 14,

  // The ability.
  kPreviousAbility = 15,
  kNextAbility = 16,

  // The current move.
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

  // The status conditions.
  kStatusId = 30,
  /// The packed data of the status condition (turns, duration...).
  kStatusData = 31,
  /// A major status condition or a volatile status condition.
  kConditionCategory = 32,

  kAmount = 33, ///< A quantity (HP that changes, HP that heal...).
  /// true when the Pokémon is in the sky or underground (Fly, Dig...).
  kIsSemiInvulnerable = 34,
  kFailureReason = 35,
  /// The damage of the confusion. The game never reads it.
  kConfusionDamage = 36,
  kTurnCount = 37,

  // The secondary effect and the accuracy.
  /// The chance of the secondary effect in the data of the move, in percent.
  kDefaultEffectChance = 38,
  /// The chance of the secondary effect now, in percent (Serene Grace
  /// doubles it).
  kEffectChance = 39,
  kAccuracyStage = 40,
  kEvasionStage = 41,

  // Multi-strike moves.
  kMaxHitCount = 42,
  kHitCount = 43,

  kAccuracyPercent = 44, ///< The chance to hit, in percent.
  kCriticalHitStage = 45,

  // The held item.
  kItemId = 46,
  kItemReactionEnabled = 47, ///< true when the held items can react.

  kSpeedValue = 48,

  // The damage calculation.
  kMovePower = 49,
  kMovePowerMultiplier = 50,
  kDamageAmount = 51,
  kBasePowerValue = 52,
  kDefenseValue = 53,
  kMultiplier = 54,
  kSecondaryMultiplier = 55,
  /// The damage of a fixed-damage move (Seismic Toss...).
  kFixedDamageAmount = 56,
  kTypeEffectiveness = 57,

  kWeather = 58,
  /// Why the Pokémon did not faint (Sturdy, Focus Sash, Endure...).
  kSurvivalReason = 59,
  kSwapTargetId = 60,

  kVisualEffectId = 61,
  kVisualSwapCount = 62,
  kMoveExecutionMode = 63, ///< The move can run, must fail, or must stop.
  /// The address of extra data of the moment (for example, the values of a
  /// message).
  kExtraDataPointer = 64,

  // The result flags.
  kNoEffectFlag = 65,
  kFailedFlag = 66,
  kMissedFlag = 67,
  /// Shows the failure message also when it is not a real failure.
  kShowFailureMessageFlag = 68,
  kStabFlag = 69, ///< The same-type attack bonus (STAB) applies.
  kCriticalHitFlag = 70,
  kSubstituteFlag = 71, ///< A substitute blocked the move.
  /// Shield Dust blocks the secondary effect.
  kShieldDustGuardFlag = 72,
  kSheerForceFlag = 73, ///< Sheer Force removes the secondary effect.
  kItemUsedFlag = 74, ///< The held item was used.
  kItemSwapFlag = 75, ///< An item exchange (Trick, Switcheroo).
  /// Ignores the stat stages of this Pokémon (like Unaware).
  kIgnoreStatsFlag = 76,
  /// Ignores the stat stages of all the Pokémon.
  kIgnoreAllStatsFlag = 77,
  kDelayedAttackFlag = 78,
  kMagicCoatFlag = 79, ///< Magic Coat sent the move back.
  kShowMessageFlag = 80,
  kFixedValueFlag = 81, ///< Goes with kFixedDamageAmount.
  kBerryFlag = 82,
  kCheckResultFlag = 83, ///< A shared yes or no result for many checks.
  kBurnPreventedFlag = 84,

  kFieldSide = 85,
  kTeamEffectId = 86,
  kAuraBreakFlag = 87,
  kAuraPendingFlag = 88, ///< An aura effect waits to apply.
  kEffectNumber = 89, ///< The number of a visual or mechanical effect.

  kCount = 90,
};
}