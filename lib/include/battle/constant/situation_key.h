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
enum class SituationKey : u8 {
  kNone = 0, ///< No value set / sentinel
  kScopeBoundary = 1,
  ///< Internal marker separating nested scopes (written by Situation_Begin, never read directly)

  // Who's involved
  kPokemonId = 2, ///< The Pokémon this handler concerns
  kMoveUserId = 3, ///< The Pokémon performing the move being resolved right now
  kMoveRecipientId = 4, ///< The Pokémon on the receiving end of that move
  kTargetCount = 5, ///< How many Pokémon are targeted
  kTargetId1 = 6, kTargetId2 = 7, kTargetId3 = 8,
  kTargetId4 = 9, kTargetId5 = 10, kTargetId6 = 11,

  // The chosen action
  kActionKind = 12, ///< Which kind of action was chosen (fight/item/switch...)
  kFieldPosition = 13, ///< Current field slot
  kOriginalFieldPosition = 14, ///< Field slot before this action started

  // Ability
  kPreviousAbility = 15,
  kNextAbility = 16,

  // The move being used
  kSpecialPriorityBonus = 17,
  ///< Extra priority granted outside the move's own (Quick Claw-style)
  kMoveId = 18,
  kMoveEffectId = 19, ///< Secondary effect this move can inflict
  kOriginalMoveId = 20,
  ///< Move actually chosen, before substitution (Metronome, Sleep Talk...)
  kTypeBeingChecked = 21,
  ///< The specific type currently being tested (a Pokémon can have several)
  kMoveType = 22,
  kMoveTypeOverride = 23,
  ///< Type actually used if different from the move's own (Hidden Power, Judgment...)
  kMoveSlotIndex = 24,
  kMovePriority = 25,
  kMoveUseSerial = 26, ///< Unique id for this particular use of the move
  kDamageCategory = 27, ///< Physical / Special / Status
  kMoveTargetingRule = 28,
  ///< How targets are selected (single, spread, self, field...)
  kMoveUserType = 29, ///< The move user's own type, cached for STAB

  // Status conditions
  kStatusId = 30,
  kStatusData = 31, ///< Packed status data (turn counter, permanence flag...)
  kConditionCategory = 32, ///< Major status vs volatile status

  kAmount = 33, ///< Generic quantity (HP shifted, healed...)
  kIsSemiInvulnerable = 34, ///< Pokémon currently vanished (Fly/Dig-style)
  kFailureReason = 35,
  kTurnCount = 36,

  // Accuracy / evasion
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
  kItemReactionEnabled = 46,
  ///< Whether held-item effects are allowed to trigger

  kSpeedValue = 47,

  // Damage calculation
  kMovePower = 48,
  kMovePowerMultiplier = 49,
  kDamageAmount = 50,
  kBasePowerValue = 51,
  kDefenseValue = 52,
  kMultiplier = 53,
  kSecondaryMultiplier = 54,
  kFixedDamageAmount = 55, ///< Used by fixed-damage moves (Seismic Toss...)
  kTypeEffectiveness = 56,

  kWeather = 57,
  kSurvivalReason = 58,
  ///< Why the Pokémon didn't faint (Sturdy, Focus Sash, Endure...)
  kSwapTargetId = 59, ///< Pokémon being swapped with

  kVisualEffectId = 60,
  kVisualSwapCount = 61,
  kMoveExecutionMode = 62,
  ///< Whether the move is allowed to run, forced to fail, or forced to stop
  kExtraDataPointer = 63,
  ///< Raw pointer to auxiliary data passed alongside a moment (e.g. a custom message's parameters)

  // Result flags
  kNoEffectFlag = 64,
  kFailedFlag = 65,
  kMissedFlag = 66,
  kShowFailureMessageFlag = 67,
  ///< Show the failure message even though this isn't counted as a real failure
  kStabFlag = 68, ///< Same-Type Attack Bonus applies
  kCriticalHitFlag = 69,
  kSubstituteFlag = 70, ///< Blocked by Substitute
  kOvercoatGuardFlag = 71, ///< Overcoat's weather/powder immunity
  kSheerForceFlag = 72, ///< Sheer Force's secondary-effect suppression
  kItemSwapFlag = 73, ///< Trick/Switcheroo-style item swap
  kResetStatsFlag = 74, ///< Flatten this Pokémon's stat stages
  kResetAllStatsFlag = 75, ///< Flatten every Pokémon's stat stages (Haze-style)
  kDelayedAttackFlag = 76,
  kMagicCoatFlag = 77, ///< Reflected back by Magic Coat
  kShowMessageFlag = 78,
  kFixedValueFlag = 79, ///< Pairs with kFixedDamageAmount
  kBerryFlag = 80,
  kCheckResultFlag = 81,
  ///< Shared yes/no result slot reused by many different checks
  kBurnPreventedFlag = 82,

  kFieldSide = 83,
  kTeamEffectId = 84,
  kAuraBreakFlag = 85,
  kAuraPendingFlag = 86, ///< Aura effect queued to apply
  kEffectNumber = 87, ///< Generic visual/mechanical effect index

  kCount = 88,
};
}