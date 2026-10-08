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
 * @file mutation_kind.h
 * @brief The kinds of mutations: the changes that a reaction asks for.
 */

#pragma once

#include <types.h>

namespace battle {
/// The kind of a mutation. See battle::Controller::Create().
enum class MutationKind : u8 {
  kUseItem = 0, ///< Uses a held item or an item of the Bag.
  kShowAbilityBanner = 1, ///< Shows the banner of the ability.
  kHideAbilityBanner = 2, ///< Hides the banner of the ability.
  kShowMessage = 3,

  kRecoverHp = 4,
  kLifestealHeal = 5, ///< Restores HP with a part of the damage of the move.
  kDealDamage = 6,
  /// Changes the HP without damage and without healing (like Pain Split).
  kAdjustHpDirectly = 7,
  kRecoverPp = 8,
  kReducePp = 9,

  kCureStatus = 10,
  kInflictStatus = 11,
  /// Changes the parameters of a status condition that the target has.
  kSetStickyStatusParams = 12,

  kAdjustStatStage = 13,
  kSetStatStageDirectly = 14,
  kResetAllStatStages = 15,
  kOverwriteBaseStat = 16, ///< Replaces a stat value (Attack, Defense...).
  kRemoveStatDebuffs = 17,

  kKnockOut = 18,
  kChangeType = 19,
  kAddExtraType = 20, ///< Gives a third type to the Pokémon.

  /// Sets a marker that the engine clears at the end of the turn.
  kSetTurnMarker = 21,
  kClearTurnMarker = 22, ///< Clears a turn marker.
  kSetPersistentMarker = 23, ///< Sets a marker that stays between the turns.
  kClearPersistentMarker = 24,

  /// Adds an effect on one side of the battlefield (Light Screen, Tailwind...).
  kAddTeamEffect = 25,
  kRemoveTeamEffect = 26,
  kSetTeamEffectPaused = 27,

  /// Adds an effect on the full battlefield (Trick Room, Gravity...).
  kAddFieldEffect = 28,
  kRemoveFieldEffect = 29,
  kChangeWeather = 30,
  /// Adds an effect on one position of the battlefield.
  kAddPositionalEffect = 31,

  kChangeAbility = 32,
  kSetHeldItem = 33,
  kCheckItemActivation = 34, ///< Checks if the effect of a held item occurs.
  kActivateItemEffect = 35,
  kConsumeItem = 36,
  kSwapHeldItems = 37,

  kOverwriteMoveData = 38,
  /// Sets a move counter of a Pokémon (for combination moves and
  /// counterattacks).
  kSetMoveCounter = 39,
  /// Gives damage in a later turn (like Future Sight).
  kDelayedMoveDamage = 40,

  kLeaveBattle = 41,
  kSwitchInPokemon = 42,
  kBatonTouch = 43, ///< Gives the stat stages to the next Pokémon (Baton Pass).
  kFlinch = 44,
  kRevive = 45,
  kSetWeight = 46,
  /// Makes the Pokémon leave the battle (like Roar or Whirlwind).
  kForceSwitchOut = 47,
  /// Makes a Pokémon act now, outside of the turn order.
  kForceActImmediately = 48,
  kInterceptPendingMove = 49, ///< Stops a Pokémon that is about to use a move.
  kDeferActionToTurnEnd = 50, ///< Makes a Pokémon act last in the turn.
  kSwapActivePokemon = 51,

  kTransform = 52,
  kBreakIllusion = 53,
  kCheckGravityEffects = 54, ///< Runs the checks of Gravity.
  /// Brings a Pokémon back from the sky or from underground (Fly, Dig,
  /// Dive...).
  kCancelSemiInvulnerableState = 55,

  /// Plays a visual effect at a position of the battlefield.
  kPlayVisualEffectAtPosition = 56,
  kFadeOutMessageWindow = 57,
  kChangeForm = 58,
  /// Selects the visual variant of the effect of a move.
  kSetMoveEffectVariant = 59,
  kForcePlayMoveEffect = 60, ///< Always plays the visual effect of a move.
  kFreefall = 61,
  kApplyFriendshipBonus = 62,

  kCount = 63,
};
}