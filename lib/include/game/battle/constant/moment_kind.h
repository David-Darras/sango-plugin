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
enum class MomentKind : u16 {
  kNone = 0,
  kActionStart = 1, ///< An individual queued action begins processing
  kActionEnd = 2, ///< An individual queued action finishes processing
  kMoveSequenceStart = 3, ///< The move-resolution sequence begins
  kMoveSequenceEnd = 4, ///< The move-resolution sequence ends
  kSubstitutePierceCheck = 5,
  ///< Check whether the move pierces through Substitute
  kCheckDelayedMoveReady = 6,
  ///< Check whether a delayed move (Future Sight-style) is ready to prepare
  kDelayedMoveReadyConfirmed = 7, ///< A delayed move has finished preparing
  kMoveStealConfirmed = 8, ///< A move-steal (Snatch-style) has been confirmed
  kMoveReflectConfirmed = 9,
  ///< A move-reflect (Magic Coat-style) has been confirmed
  kSkipEscapeOddsCheck = 11,
  ///< Check whether to skip the escape-chance calculation
  kEscapeForbiddenCheck = 12, ///< Check whether escaping is blocked
  kEscapeSpecialMessage = 13,
  ///< Provide a special message for a failed/blocked escape
  kDrowsinessCheck = 14, ///< Check success/failure of a "dozing off" effect
  kSpecialPriorityCheck = 15,
  ///< Check for a special (non-standard) priority boost
  kSpecialPriorityApplied = 16,
  ///< A special priority effect actually triggered
  kGetMovePriority = 17, ///< Retrieve the move's priority
  kLevitationCheck = 18,
  ///< Check for a Levitate-style immunity to ground effects
  kCalculateSpeed = 19, ///< Calculate effective Speed
  kBeforeFirstMoveOfTurn = 21,
  ///< Right before the turn's very first move sequence
  kRequestMoveForTurnOrder = 22,
  ///< Query an alternate move used just to compute turn order
  kRequestedMoveParams = 24,
  ///< Parameters for a move requested/substituted on the fly
  kRequestedMoveMessage = 25,
  ///< Message shown when a move is requested/substituted on the fly
  kMoveStealCheck = 26, ///< Check whether a move-steal effect applies
  kGroundMoveBlockedByLevitation = 27,
  ///< A ground-type move is nullified by a Levitate-style state
  kSkipAccuracyCheck = 28,
  ///< Check whether to skip the hit/miss calculation entirely
  kIgnoreMoveFailureCause = 29,
  ///< Decide whether to ignore what would normally cause the move to fail
  kMoveExecutionCheckEarly = 30,
  ///< Move-execution check, before the move message and before confusion's own check
  kMoveExecutionCheckMid = 31,
  ///< Move-execution check, before the move message but after confusion's own check
  kMoveFailedToExecute = 33, ///< The move failed to go off
  kMoveAnnouncementConfirmed = 34,
  ///< The move's name has been declared and its PP consumed
  kMoveExecutionConfirmed = 35,
  ///< Execution is confirmed (not blocked by Taunt/Torment/etc.)
  kMoveExecutionStart = 36, ///< The move's execution processing begins
  kMoveSucceededWithEffect = 37, ///< The move went off and had an effect
  kMoveSucceededNoEffect = 38, ///< The move went off but had no effect
  kMoveExecutionEnd = 39, ///< The move's execution processing ends
  kMoveParamCheck = 40, ///< Check the move's parameters
  kMoveParamCheckSecondPass = 41, ///< A second pass of move parameter checks
  kMoveTargetDecided = 42, ///< The move's target(s) are decided
  kRedirectTargetToSelf = 43,
  ///< Redirect the move's target onto oneself (Follow Me-style)
  kImmunityCheckBegin = 44, ///< Move-nullification check sequence begins
  kImmunityCheckTier1 = 45,
  ///< Immunity check tier 1 (loses to an always-hit effect)
  kImmunityCheckTier2 = 46,
  ///< Immunity check tier 2 (beats even an always-hit effect)
  kProtectCheck = 47, ///< Immunity check equivalent to Protect
  kImmunityCheckTier3 = 48,
  ///< Tier 3 (beats always-hit, after Protect, before type-immunity check)
  kImmunityCheckTier4 = 49,
  ///< Tier 4 (beats always-hit, after Protect, after type-immunity check)
  kImmunityCheckEnd = 50, ///< Move-nullification check sequence ends
  kProtectBypassCheck = 51, ///< Check whether an effect bypasses Protect
  kDamageToHealConversionCheck = 53,
  ///< Check whether this move's damage should convert into healing instead
  kDamageToHealConversionConfirmed = 54,
  ///< Confirms the damage-to-healing conversion
  kSkipAccuracyCalculation = 55,
  ///< Check whether to skip the accuracy-percentage calculation
  kAccuracyEvasionStageDecision = 56,
  ///< Decide the accuracy/evasion stages used
  kAccuracyModifier = 57, ///< Adjust the move's accuracy
  kMultiHitCountDecision = 58, ///< Decide the hit count for a multi-hit move
  kCriticalHitCheck = 59, ///< Check for a critical hit
  kMoveBasePower = 60, ///< The move's base power
  kMovePowerModifier = 61, ///< Adjust the move's power
  kBeforeAttackerOffenseStat = 62,
  ///< Right before reading the attacker's offense stat (Attack or Sp. Atk)
  kBeforeDefenderDefenseStat = 63,
  ///< Right before reading the defender's defense stat (Defense or Sp. Def)
  kAttackerOffenseStatModifier = 64, ///< Adjust the attacker's offense stat
  kDefenderDefenseStatModifier = 65, ///< Adjust the defender's defense stat
  kContinueDespiteNoTargetsLeft = 66,
  ///< Decide whether to keep resolving the move even if no targets remain
  kTypeEffectivenessCheckEnabled = 67,
  ///< Decide whether the type-effectiveness check should run at all
  kCalculateTypeEffectiveness = 68, ///< Calculate type effectiveness
  kOverrideTypeEffectiveness = 69,
  ///< Force-overwrite the calculated type effectiveness
  kSkipLevitationInTypeCheck = 70,
  ///< Skip checking Levitate-style state during the type-effectiveness calc
  kStabCheck = 72, ///< Check whether STAB applies
  kStabMultiplier = 73, ///< The STAB multiplier applied
  kRightAfterDamageAnimation = 76, ///< Right after the damage animation plays
  kDamageWillLandConfirmed = 77,
  ///< Confirms a damaging move will land, before damage is calculated
  kDamageModifierBeforeTypeCalc = 78,
  ///< Damage adjustment before the type-effectiveness multiplier
  kDamageModifierAfterTypeCalc = 79,
  ///< Damage adjustment after the type-effectiveness multiplier
  kDamageCalculationFinal = 80, ///< Final step of the damage calculation
  kBeforeDamageReactions = 82, ///< Right before per-target damage reactions run
  kDamageReaction = 83, ///< A single target's reaction to taking damage
  kDamageReactionSecondPass = 84,
  ///< A second pass of per-target damage reactions
  kAfterDamagingAllTargets = 85,
  ///< Runs once, after damage has been dealt to every target
  kGetPpCostForThisUse = 86,
  ///< Retrieve how much PP this use of the move should cost
  kAfterPpDeducted = 87, ///< Runs after the move's PP has been deducted
  kRecoilCalculation = 88, ///< Calculate recoil damage
  kAdditionalStatEffectOnTarget = 89,
  ///< The move's secondary stat-stage effect on its target
  kSwitchInterrupt = 91,
  ///< A party-member switch interrupts the current action
  kAfterPokemonWithdrawn = 92,
  ///< Right after a specific Pokémon has been withdrawn
  kBatonTouchHandoff = 93,
  ///< Right after a Pokémon enters via Baton Touch, carrying over stat stages
  kPokemonEntered = 94, ///< Right after a specific Pokémon enters the field
  kBeforeAllPokemonEnter = 95,
  ///< Right before the "all Pokémon enter" event sequence
  kAfterAllPokemonEnter = 96,
  ///< After the "all Pokémon enter" event sequence has processed
  kAfterRotationForBothSides = 97,
  ///< After both sides' Rotation Battle turn has resolved
  kStatStageDeltaFromMove = 98,
  ///< Check the stat-stage increase/decrease amount from a move
  kFinalStatStageDeltaCheck = 99,
  ///< Final check on the stat-stage change amount
  kStatStageChangeOutcomeCheck = 100,
  ///< Final success/failure check for a stat-stage change
  kStatStageChangeFailed = 101, ///< The stat-stage change is confirmed to fail
  kAfterStatStageChange = 102,
  ///< Runs after a stat-stage change has been applied
  kMoveStatStageEffectSucceeded = 103, ///< A move's stat-stage effect succeeded
  kSpecialStatusIdDecision = 105,
  ///< Decide the specific status id for a specially-handled status effect
  kStandardStatusMessage = 106,
  ///< The text shown when a move inflicts a standard status condition
  kMoveInflictedStatusParamCheck = 107,
  ///< Check the parameters of a status condition inflicted by a move
  kMoveSecondaryStatusCheck = 109,
  ///< Check whether a move's secondary status effect triggers
  kInflictStatusFailureCheck = 110,
  ///< Check whether inflicting the status condition fails
  kInflictStatusFailed = 112,
  ///< Inflicting the status condition is confirmed to fail
  kMajorStatusConfirmed = 113, ///< A major status condition has been confirmed
  kMoveInflictedStatusConfirmed = 114,
  ///< A move-inflicted status condition has been confirmed
  kAbilitySuppressionConfirmed = 115,
  ///< Gastro Acid: confirms an ability has been suppressed
  kStatusConditionDamage = 116, ///< Damage calculation from a status condition
  kFlinchChanceFromMove = 117, ///< Calculate a move's flinch chance
  kFlinchCheck = 118, ///< Check whether flinching occurs
  kFlinchFailed = 119, ///< Flinching is confirmed to fail
  kOneHitKoCheck = 121, ///< Check for a one-hit-KO move
  kUseHeldItem = 123, ///< A held item is used
  kUseHeldItemTemporary = 124,
  ///< A held item is used (temporary/one-off processing)
  kEndureCheck = 125,
  ///< Check for a "hang on at 1 HP" effect (Sturdy/Focus Sash/Endure)
  kEndureTriggered = 126, ///< The "hang on at 1 HP" effect triggers
  kEndOfTurnChecksBegin = 127, ///< End-of-turn checks begin
  kEndOfTurnChecksEnd = 128, ///< End-of-turn checks end
  kAfterEndOfTurnChecks = 129, ///< Runs after end-of-turn checks have completed
  kAirLockActivated = 130,
  ///< Air Lock/Cloud Nine-style weather-neutralizing ability activates
  kWeatherCheck = 131, ///< Check the current weather
  kWeightMultiplierCheck = 132, ///< Check a weight-change multiplier
  kMoveWeatherDurationCheck = 133,
  ///< Check how many turns a move's weather change should last
  kBeforeWeatherChanges = 134, ///< Right before the weather changes
  kAfterWeatherChanges = 135, ///< Right after the weather changes
  kWeatherDamageReaction = 136, ///< Reaction to weather-based damage
  kNonMoveDamageEnabled = 137,
  ///< Check whether non-move damage is allowed to apply
  kDamageSequenceStart = 138, ///< The damaging-move sequence begins
  kBeforeDamageSequenceEnd = 139,
  ///< Right before the damaging-move sequence ends
  kDamageSequenceEndRealHit = 140,
  ///< Sequence end, at least one Pokémon (excluding Substitute) actually took damage
  kDamageSequenceEndHitTier1 = 141,
  ///< Sequence end tier 1, at least one Pokémon (Substitute included) took damage
  kDamageSequenceEndHitTier2 = 142, ///< Sequence end tier 2, same condition
  kDamageSequenceEndHitTier3 = 143, ///< Sequence end tier 3, same condition
  kDamageSequenceEndHitTier4 = 144, ///< Sequence end tier 4, same condition
  kDamageSequenceEnd = 145, ///< The damaging-move sequence ends (always called)
  kNonDamageSequenceEnd = 146,
  ///< The non-damaging-move sequence ends (always called)
  kBeforeAbilityChange = 147, ///< Right before an ability change is confirmed
  kAfterAbilityChange = 148, ///< Right after an ability change
  kForceSwitchMoveCheck = 149,
  ///< Check for a forced-switch-style move (Roar/Whirlwind)
  kLifestealAmountCalculation = 150,
  ///< Calculate the amount healed by a lifesteal-style move
  kLifestealAmountFinal = 151, ///< Final lifesteal amount
  kHealMovePercentage = 153, ///< Calculate a healing move's recovery percentage
  kHeldItemUsageFailureCheck = 154,
  ///< Check whether using the held item should fail
  kAfterHeldItemUsed = 155, ///< Runs after the held item is used
  kItemReactionCheck = 156,
  ///< Check whether a held item reacts to the current situation
  kChargingTurnFailureCheck = 158,
  ///< Check for failure during a move's charging turn
  kChargingTurnSkipCheck = 159,
  ///< Check whether the charging turn can be skipped
  kChargingStart = 160, ///< A charging turn begins
  kChargingStartConfirmed = 161, ///< The charging turn start is confirmed
  kChargingSkipConfirmed = 162, ///< Skipping the charging turn is confirmed
  kChargeRelease = 163, ///< The charged move is unleashed
  kChargeReleaseConfirmed = 164, ///< The charge release is confirmed
  kSemiInvulnerableHitCheck = 165,
  ///< Check move accuracy against a semi-invulnerable (Fly/Dig-style) Pokémon
  kItemOverwriteFailureCheck = 166,
  ///< Check whether overwriting the held item should fail
  kItemOverwriteFailed = 167,
  ///< Overwriting the held item is confirmed to fail
  kItemOverwriteConfirmed = 168, ///< Overwriting the held item is confirmed
  kAfterItemOverwrite = 169, ///< Runs after the held item has been overwritten
  kFieldEffectTrigger = 170,
  ///< A field effect (Trick Room, Gravity...) is added
  kTeamEffectParamAdjustment = 171, ///< Adjust a team-side effect's parameters
  kUncategorizedMoveEffect = 172,
  ///< Catch-all processing for a move's uncategorized effect
  kUncategorizedMoveEffectNoTarget = 173, ///< Same, for moves without a target
  kComboMoveCheck = 174,
  ///< Check whether a combination move (e.g. an elemental Pledge combo) triggers
  kRightBeforeFainting = 175, ///< Right before the Pokémon faints
  kAfterMoveAction = 176, ///< Right after a move action resolves
  kProtectSucceeded = 177,
  ///< A Protect-style effect successfully blocked the opponent's move
  kFreeFallChargeGuardCheck = 178,
  ///< Check whether to guard against a Sky Drop-style charging turn
  kForceSwitchVisualEffectId = 179,
  ///< Decide the visual effect id for a forced-switch move
  kCheckAttackerFaintsBeforeTarget = 180,
  ///< Decide whether the attacker's own fainting is checked before its target's
  kSpecialSecondaryEffectChance = 181,
  ///< Check a special move's secondary-effect chance
  kChargeReleaseFailed = 182, ///< The charged move's release fails
  kForcedFaintTriggered = 183, ///< A forced-faint effect has been invoked
  kPokemonEnteredAfterMegaEvolution = 184,
  ///< entry moment used to skip the held-item check
  kBeforeFormOrTransformChange = 185,
  ///< Right before Transform/Mega Evolution/form change (cancels weather-summoning abilities)
  kCount = 186,
};
}