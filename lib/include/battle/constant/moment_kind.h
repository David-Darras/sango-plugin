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
 * @file moment_kind.h
 * @brief The moments of the battle engine.
 *
 * A moment is a time in a battle when the engine calls the reactions of the
 * listeners. See docs/concepts/battle-engine.md.
 */

#pragma once
#include <types.h>

namespace battle {
/// A moment of the battle engine.
enum class MomentKind : u16 {
  kNone = 0,
  kActionStart = 1,
  kActionEnd = 2,
  kMoveSequenceStart = 3,
  kMoveSequenceEnd = 4,
  kSubstitutePierceCheck = 5, ///< Checks if the move goes through a substitute.
  /// Checks if a delayed move (like Future Sight) is ready.
  kCheckDelayedMoveReady = 6,
  kDelayedMoveReadyConfirmed = 7,
  kMoveStealConfirmed = 8, ///< A move steal (like Snatch) is confirmed.
  /// A move reflection (like Magic Coat) is confirmed.
  kMoveReflectConfirmed = 9,
  /// Checks if the escape chance calculation is skipped.
  kSkipEscapeOddsCheck = 11,
  kEscapeForbiddenCheck = 12,
  /// Gives a special message when the escape fails.
  kEscapeSpecialMessage = 13,
  /// Checks if a drowsiness effect (like Yawn) succeeds.
  kDrowsinessCheck = 14,
  kSpecialPriorityCheck = 15, ///< Checks for a special priority increase.
  kSpecialPriorityApplied = 16, ///< A special priority effect occurs.
  kGetMovePriority = 17,
  /// Checks for an immunity to Ground moves (like Levitate).
  kLevitationCheck = 18,
  kCalculateSpeed = 19,
  kBeforeFirstMoveOfTurn = 21, ///< Before the first move of the turn.
  /// Asks for a different move to calculate the turn order.
  kRequestMoveForTurnOrder = 22,
  /// The parameters of a move that replaces the selected move.
  kRequestedMoveParams = 24,
  /// The message of a move that replaces the selected move.
  kRequestedMoveMessage = 25,
  kMoveStealCheck = 26,
  /// A Ground move has no effect because of a levitation.
  kGroundMoveBlockedByLevitation = 27,
  /// Checks if the hit or miss calculation is skipped.
  kSkipAccuracyCheck = 28,
  /// Decides if the move ignores a cause of failure.
  kIgnoreMoveFailureCause = 29,
  /// Checks the move before its message and before the confusion check.
  kMoveExecutionCheckEarly = 30,
  /// Checks the move before its message and after the confusion check.
  kMoveExecutionCheckMid = 31,
  kMoveFailedToExecute = 33,
  /// The game showed the move name and removed the PP.
  kMoveAnnouncementConfirmed = 34,
  /// The move can start (no Taunt, no Torment...).
  kMoveExecutionConfirmed = 35,
  kMoveExecutionStart = 36,
  kMoveSucceededWithEffect = 37,
  kMoveSucceededNoEffect = 38,
  kMoveExecutionEnd = 39,
  kMoveParamCheck = 40,
  kMoveParamCheckSecondPass = 41,
  kMoveTargetDecided = 42,
  /// Sends the move to the listener's Pokémon (like Follow Me).
  kRedirectTargetToSelf = 43,
  kImmunityCheckBegin = 44,
  /// Immunity check, tier 1. A sure-hit effect wins over it.
  kImmunityCheckTier1 = 45,
  /// Immunity check, tier 2. It wins over a sure-hit effect.
  kImmunityCheckTier2 = 46,
  kProtectCheck = 47, ///< Immunity check of Protect.
  /// Immunity check, tier 3: after Protect, before the type immunity.
  kImmunityCheckTier3 = 48,
  /// Immunity check, tier 4: after Protect, after the type immunity.
  kImmunityCheckTier4 = 49,
  kImmunityCheckEnd = 50,
  kProtectBypassCheck = 51,
  /// Checks if the damage of the move heals instead.
  kDamageToHealConversionCheck = 53,
  kDamageToHealConversionConfirmed = 54, ///< The damage heals instead.
  /// Checks if the accuracy calculation is skipped.
  kSkipAccuracyCalculation = 55,
  /// Decides the accuracy stage and the evasion stage.
  kAccuracyEvasionStageDecision = 56,
  kAccuracyModifier = 57,
  kMultiHitCountDecision = 58,
  kCriticalHitCheck = 59,
  kMoveBasePower = 60,
  kMovePowerModifier = 61,
  /// Before the game reads the Attack or the Sp. Atk of the attacker.
  kBeforeAttackerOffenseStat = 62,
  /// Before the game reads the Defense or the Sp. Def of the target.
  kBeforeDefenderDefenseStat = 63,
  kAttackerOffenseStatModifier = 64,
  kDefenderDefenseStatModifier = 65,
  /// Decides if the move continues when no target remains.
  kContinueDespiteNoTargetsLeft = 66,
  /// Decides if the type effectiveness check runs.
  kTypeEffectivenessCheckEnabled = 67,
  kCalculateTypeEffectiveness = 68,
  /// Replaces the calculated type effectiveness.
  kOverrideTypeEffectiveness = 69,
  /// Skips the levitation check in the type calculation.
  kSkipLevitationInTypeCheck = 70,
  kStabCheck = 72,
  kStabMultiplier = 73,
  kRightAfterDamageAnimation = 76,
  /// A damaging move hits, before the damage calculation.
  kDamageWillLandConfirmed = 77,
  /// Changes the damage before the type multiplier.
  kDamageModifierBeforeTypeCalc = 78,
  /// Changes the damage after the type multiplier.
  kDamageModifierAfterTypeCalc = 79,
  kDamageCalculationFinal = 80,
  kBeforeDamageReactions = 82,
  kDamageReaction = 83, ///< One target reacts to the damage.
  /// The targets react to the damage a second time.
  kDamageReactionSecondPass = 84,
  /// After the damage to all the targets (one time).
  kAfterDamagingAllTargets = 85,
  kGetPpCostForThisUse = 86, ///< Gives the PP cost of this use of the move.
  kAfterPpDeducted = 87,
  kRecoilCalculation = 88,
  /// The secondary stat change of the move on its target.
  kAdditionalStatEffectOnTarget = 89,
  kSwitchInterrupt = 91, ///< A switch stops the current action.
  kAfterPokemonWithdrawn = 92, ///< After a Pokémon leaves the battle.
  /// A Pokémon enters with Baton Pass and gets the stat stages.
  kBatonTouchHandoff = 93,
  kPokemonEntered = 94,
  kBeforeAllPokemonEnter = 95, ///< Before all the Pokémon enter the battle.
  kAfterAllPokemonEnter = 96, ///< After all the Pokémon enter the battle.
  /// After the rotation of the two sides (Rotation Battle).
  kAfterRotationForBothSides = 97,
  kStatStageDeltaFromMove = 98, ///< Checks the stat stage change of a move.
  kFinalStatStageDeltaCheck = 99, ///< The last check of the stat stage change.
  /// Checks if the stat stage change succeeds.
  kStatStageChangeOutcomeCheck = 100,
  kStatStageChangeFailed = 101,
  kAfterStatStageChange = 102, ///< After a stat stage change.
  kMoveStatStageEffectSucceeded = 103,
  /// Decides the id of a special status condition.
  kSpecialStatusIdDecision = 105,
  /// The message of a standard status condition from a move.
  kStandardStatusMessage = 106,
  /// Checks the parameters of a status condition from a move.
  kMoveInflictedStatusParamCheck = 107,
  /// Checks if the secondary status condition of a move occurs.
  kMoveSecondaryStatusCheck = 109,
  kInflictStatusFailureCheck = 110, ///< Checks if the status condition fails.
  kInflictStatusFailed = 112, ///< The status condition fails.
  kMajorStatusConfirmed = 113,
  /// A status condition from a move is confirmed.
  kMoveInflictedStatusConfirmed = 114,
  kAbilitySuppressionConfirmed = 115, ///< Gastro Acid: an ability stops.
  kStatusConditionDamage = 116,
  kFlinchChanceFromMove = 117,
  kFlinchCheck = 118,
  kFlinchFailed = 119,
  kOneHitKoCheck = 121,
  kUseHeldItem = 123,
  kUseHeldItemTemporary = 124, ///< A held item is used one time only.
  /// Checks for an effect that keeps 1 HP (Sturdy, Focus Sash, Endure).
  kEndureCheck = 125,
  kEndureTriggered = 126, ///< An effect keeps the Pokémon at 1 HP.
  kEndOfTurnChecksBegin = 127,
  kEndOfTurnChecksEnd = 128,
  kAfterEndOfTurnChecks = 129,
  /// An ability stops the weather effects (Air Lock, Cloud Nine).
  kAirLockActivated = 130,
  kWeatherCheck = 131,
  kWeightMultiplierCheck = 132,
  /// Checks the number of turns of a weather from a move.
  kMoveWeatherDurationCheck = 133,
  kBeforeWeatherChanges = 134,
  kAfterWeatherChanges = 135,
  kWeatherDamageReaction = 136,
  kNonMoveDamageEnabled = 137, ///< Checks if damage without a move can occur.
  kDamageSequenceStart = 138,
  kBeforeDamageSequenceEnd = 139, ///< Before the end of a damaging move.
  /// End of a damaging move: a Pokémon (not a substitute) took damage.
  kDamageSequenceEndRealHit = 140,
  /// End of a damaging move, tier 1: a Pokémon or a substitute took damage.
  kDamageSequenceEndHitTier1 = 141,
  /// End of a damaging move, tier 2: same condition.
  kDamageSequenceEndHitTier2 = 142,
  /// End of a damaging move, tier 3: same condition.
  kDamageSequenceEndHitTier3 = 143,
  /// End of a damaging move, tier 4: same condition.
  kDamageSequenceEndHitTier4 = 144,
  /// End of a damaging move. The engine always calls it.
  kDamageSequenceEnd = 145,
  /// End of a move without damage. The engine always calls it.
  kNonDamageSequenceEnd = 146,
  kBeforeAbilityChange = 147,
  kAfterAbilityChange = 148,
  /// Checks for a move that forces a switch (Roar, Whirlwind).
  kForceSwitchMoveCheck = 149,
  /// Calculates the HP that an HP drain move restores.
  kLifestealAmountCalculation = 150,
  kLifestealAmountFinal = 151,
  kHealMovePercentage = 153,
  /// Checks if the use of the held item fails.
  kHeldItemUsageFailureCheck = 154,
  kAfterHeldItemUsed = 155,
  kItemReactionCheck = 156, ///< Checks if a held item reacts to the situation.
  /// Checks for a failure during the charge turn of a move.
  kChargingTurnFailureCheck = 158,
  kChargingTurnSkipCheck = 159, ///< Checks if the charge turn is skipped.
  kChargingStart = 160,
  kChargingStartConfirmed = 161,
  kChargingSkipConfirmed = 162,
  kChargeRelease = 163,
  kChargeReleaseConfirmed = 164,
  /// Checks a hit on a Pokémon that is in the sky or underground.
  kSemiInvulnerableHitCheck = 165,
  /// Checks if the change of the held item fails.
  kItemOverwriteFailureCheck = 166,
  kItemOverwriteFailed = 167, ///< The change of the held item fails.
  kItemOverwriteConfirmed = 168,
  kAfterItemOverwrite = 169,
  /// A battlefield effect (Trick Room, Gravity...) starts.
  kFieldEffectTrigger = 170,
  kTeamEffectParamAdjustment = 171,
  kUncategorizedMoveEffect = 172, ///< The effect of a move without a category.
  /// The same, for a move without a target.
  kUncategorizedMoveEffectNoTarget = 173,
  /// Checks if a combination move (like a Pledge combination) occurs.
  kComboMoveCheck = 174,
  kRightBeforeFainting = 175,
  kAfterMoveAction = 176,
  /// A protection move blocked the move of the opponent.
  kProtectSucceeded = 177,
  /// Checks the protection during the charge turn of Sky Drop.
  kFreeFallChargeGuardCheck = 178,
  /// Decides the visual effect of a move that forces a switch.
  kForceSwitchVisualEffectId = 179,
  /// Decides if the attacker faints before its target.
  kCheckAttackerFaintsBeforeTarget = 180,
  /// Checks the secondary effect chance of a special move.
  kSpecialSecondaryEffectChance = 181,
  kChargeReleaseFailed = 182,
  kForcedFaintTriggered = 183,
  /// The Pokémon enters after a Mega Evolution. The held item check is skipped.
  kPokemonEnteredAfterMegaEvolution = 184,
  /// Before Transform, a Mega Evolution or a form change. It stops the weather
  /// abilities.
  kBeforeFormOrTransformChange = 185,
  kCount = 186,
};
}