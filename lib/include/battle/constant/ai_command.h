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
 * @file ai_command.h
 * @brief The values that the scripts use for the AI command.
 */

#pragma once

#include <types.h>

namespace battle {

/// The AI command, as a script writes it.
enum class AiCommand : u8 {
  kRandomUnder = 0, ///< A random number is under a1.
  kRandomOver = 1, ///< A random number is over a1.
  kRandomEquals = 2, ///< A random number equals a1.
  kRandomDiffers = 3, ///< A random number does not equal a1.
  kHpPercentUnder = 4, ///< The HP of the side a1 is under a2 percent.
  kHpPercentOver = 5, ///< The HP of the side a1 is over a2 percent.
  kHpPercentEquals = 6, ///< The HP of the side a1 equals a2 percent (rounded up).
  kHpPercentDiffers = 7, ///< The HP of the side a1 does not equal a2 percent (rounded up).
  kHasStatusProblem = 8, ///< The side a1 has a status problem.
  kHasNoStatusProblem = 9, ///< The side a1 has no status problem.
  kHasSpecificStatusProblem = 10, ///< The side a1 has the status problem a2.
  kLacksSpecificStatusProblem = 11, ///< The side a1 does not have the status problem a2.
  kIsBadlyPoisoned = 12, ///< The side a1 is badly poisoned.
  kIsNotBadlyPoisoned = 13, ///< The side a1 is not badly poisoned.
  kHasLastingEffect = 14, ///< The side a1 has the lasting effect a2.
  kLacksLastingEffect = 15, ///< The side a1 does not have the lasting effect a2.
  kFieldHasSideEffect = 16, ///< The field of the side a1 has the side effect a2 (a screen, Tailwind...).
  kFieldLacksSideEffect = 17, ///< The field of the side a1 does not have the side effect a2.
  kUserHasDamagingMove = 18, ///< The user has a move that does damage.
  kUserHasNoDamagingMove = 19, ///< The user has no move that does damage.
  kGetTurn = 20, ///< Gives the number of the turn.
  kGetType = 21, ///< Gives a type. a1 chooses it: 0 first type of the target, 1 first type of the user, 2 second type of the target, 3 second type of the user, 4 type of the move.
  kRatedMoveDoesDamage = 22, ///< The move that is rated does damage (its power is not 0).
  kGetRatedMovePower = 23, ///< Gives the power of the move that is rated.
  kCompareRatedMoveDamage = 24, ///< Compares the damage of the move that is rated with the other moves of the user. a1: yes to use the variance of the damage. Gives 0 if it does no damage, 1 if another move does more, 2 if it is the strongest.
  kCompareRatedMoveDamageAgain = 25, ///< Compares the damage of the move that is rated with the other moves of the user. a1: yes to use the variance of the damage. Gives 0 if it does no damage, 1 if another move does more, 2 if it is the strongest.
  kGetLastMoveUsed = 26, ///< Gives the move that the side a1 used last.
  kGetWhoMovesFirst = 27, ///< Who moves first. a1: 0 the user is faster, 1 the target is faster, 2 same speed.
  kCountBenchAbleToFight = 28, ///< Gives the number of Pokémon on the bench of the side a1 that can still fight.
  kGetRatedMoveSequence = 29, ///< Gives the AI sequence number of the move that is rated.
  kGetAbility = 30, ///< Gives the ability of the side a1.
  kRatedMoveEffectiveness = 31, ///< The effectiveness of the move that is rated on the target. a1: the effectiveness to test (at most for the weakest, at least for the strongest, equal for the others).
  kBenchHasStatusProblem = 32, ///< A Pokémon on the bench of the side a1 has a status problem.
  kBenchHasNoStatusProblem = 33, ///< No Pokémon on the bench of the side a1 has a status problem.
  kGetWeather = 34, ///< Gives the weather: 0 none, 1 sun, 2 rain, 3 hail, 4 sandstorm.
  kStatUnder = 35, ///< The value a2 of the side a1 is under a3 (a2: 0 HP, 1 Attack, 2 Defense, 3 Sp. Atk, 4 Sp. Def, 5 Speed, 6 accuracy, 7 evasion).
  kStatOver = 36, ///< The value a2 of the side a1 is over a3 (a2: 0 HP, 1 Attack, 2 Defense, 3 Sp. Atk, 4 Sp. Def, 5 Speed, 6 accuracy, 7 evasion).
  kStatEquals = 37, ///< The value a2 of the side a1 equals a3 (a2: 0 HP, 1 Attack, 2 Defense, 3 Sp. Atk, 4 Sp. Def, 5 Speed, 6 accuracy, 7 evasion).
  kStatDiffers = 38, ///< The value a2 of the side a1 does not equal a3 (a2: 0 HP, 1 Attack, 2 Defense, 3 Sp. Atk, 4 Sp. Def, 5 Speed, 6 accuracy, 7 evasion).
  kRatedMoveFaintsTarget = 39, ///< The move that is rated makes the target faint. a1: yes to use the variance of the damage.
  kRatedMoveDoesNotFaintTarget = 40, ///< The move that is rated does not make the target faint. a1: yes to use the variance of the damage.
  kHasMove = 41, ///< The side a1 has the move a2.
  kLacksMove = 42, ///< The side a1 does not have the move a2.
  kHasMoveWithSequence = 43, ///< The side a1 has a move with the AI sequence number a2.
  kLacksMoveWithSequence = 44, ///< The side a1 has no move with the AI sequence number a2.
  kRunAway = 45, ///< Choose to run away.
  kGetHeldItem = 46, ///< Gives the held item of the side a1.
  kGetHeldItemEffect = 47, ///< Gives the effect of the held item of the side a1.
  kGetGender = 48, ///< Gives the gender of the side a1.
  kAlreadyActed = 49, ///< The side a1 already acted this turn (Fake Out check).
  kGetStockpileCount = 50, ///< Gives the Stockpile count of the side a1.
  kGetBattleRule = 51, ///< Gives the rule of the battle.
  kGetOpponentKind = 52, ///< Gives the kind of the opponent.
  kGetItemUsedUp = 53, ///< Gives the item that the side a1 used up (the one that Recycle gives back).
  kGetRatedMoveType = 54, ///< Gives the type of the move that is rated.
  kGetRatedMovePowerAgain = 55, ///< Gives the power of the move that is rated.
  kGetRatedMoveSequenceAgain = 56, ///< Gives the AI sequence number of the move that is rated.
  kGetProtectCount = 57, ///< Gives the count of Protect of the side a1.
  kCompareLevels = 58, ///< Compares the levels. a1: 0 the user is higher, 1 the target is higher, 2 same level.
  kTargetIsTaunted = 59,
  kTargetIsTauntedAgain = 60, ///< The target is taunted (the game does not invert the test here).
  kMoveAimsAtAlly = 61,
  kHasType = 62, ///< The side a1 has the type a2.
  kHasAbility = 63, ///< The side a1 has the ability a2.
  kHasFlashFirePowerUp = 64, ///< The side a1 already has the power-up of Flash Fire.
  kHoldsItem = 65, ///< The side a1 holds the item a2.
  kFieldEffectIsActive = 66, ///< The field effect a1 is active.
  kGetSideEffectCount = 67, ///< Gives the count of the side effect a2 on the side a1.
  kBenchHasLostHp = 68, ///< A Pokémon on the bench of the side a1 has lost HP.
  kBenchHasLostPp = 69, ///< A Pokémon on the bench of the side a1 has lost PP.
  kGetFlingPower = 70, ///< Gives the power of Fling with the item of the side a1 (0 if the item cannot be used).
  kGetRatedMovePpLeft = 71, ///< Gives the PP left of the move that is rated.
  kUsedAllMoves = 72, ///< The user already used all its moves (Last Resort check).
  kGetRatedMoveDamageKind = 73, ///< Gives the kind of damage of the move that is rated (1 physical, 2 special).
  kGetLastTargetMoveDamageKind = 74, ///< Gives the kind of damage of the last move of the target (1 physical, 2 special).
  kGetSpeedRank = 75, ///< Gives the speed rank of the side a1.
  kGetSlowStartTurns = 76, ///< Gives the number of turns of the side a1 (for Slow Start).
  kBenchWouldDoMoreDamage = 77, ///< A Pokémon on the bench would do more damage than the user. a1: yes to use the variance of the damage.
  kUserHasSuperEffectiveMove = 78, ///< The user has a move that is super effective on the target.
  kBestDamageBelowLastMove = 79, ///< The best damage of the user is less than the damage that the last move of the side a1 would do. a2: yes to use the variance of the damage.
  kGetRaisedStatTotal = 80, ///< Gives the total of the raised stat changes of the side a1.
  kGetStatDifference = 81, ///< Gives the difference of the value a2 between the side a1 and the user.
  kGetStat = 82, ///< Gives the value a2 of the side a1.
  kCompareRatedMoveDamageWithPartner = 83, ///< Compares the damage of the move that is rated with the other moves of the user and of its partner. a1: yes to use the variance of the damage. Gives 0 if it does no damage, 1 if another move does more, 2 if it is the strongest.
  kFainted = 84, ///< The side a1 fainted.
  kDidNotFaint = 85, ///< The side a1 did not faint.
  kGetAbilityWithMoveEffects = 86, ///< Gives the ability of the side a1 with the effects of the moves taken into account.
  kHasSubstitute = 87, ///< The side a1 has a Substitute.
  kGetSpecies = 88, ///< Gives the species of the side a1.
  kSharedRandomUnder = 89, ///< The random number that all the Pokémon share this turn is under a1.
  kSharedRandomOver = 90, ///< The random number that all the Pokémon share this turn is over a1.
  kSharedRandomEquals = 91, ///< The random number that all the Pokémon share this turn equals a1.
  kSharedRandomDiffers = 92, ///< The random number that all the Pokémon share this turn does not equal a1.
  kDelayedAttackWaiting = 93, ///< A delayed attack (Future Sight, Doom Desire) is waiting at the place of the side a1.
  kAttackUnderSpecialAttack = 94, ///< The Attack of the side a1 is under its Sp. Atk.
  kAttackOverSpecialAttack = 95, ///< The Attack of the side a1 is over its Sp. Atk.
  kAttackDiffersFromSpecialAttack = 96, ///< The Attack of the side a1 differs from its Sp. Atk (the game code tests "does not equal").
  kHasEatenBerry = 97, ///< The side a1 has eaten a Berry.
  kExtraTypeIs = 98, ///< The extra type of the side a1 is a2.
  kTerrainIsActive = 99, ///< The terrain a1 is active.
  kGetWeight = 100, ///< Gives the weight of the side a1.
  kIsMultiBattle = 101, ///< The battle is a multi battle.
  kIsMegaEvolved = 102, ///< The side a1 is Mega Evolved.
  kIsOutOfFieldByMove = 103, ///< The side a1 is out of the field because of a move (Fly, Dig...).
  kGetHiddenPowerType = 104, ///< Gives the type of Hidden Power of the side a1.
};

} // namespace battle
