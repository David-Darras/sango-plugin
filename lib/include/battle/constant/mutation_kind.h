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
enum class MutationKind : u8 {
  kUseItem = 0, ///< Use a held/bag item
  kShowAbilityBanner = 1, ///< Show the "ability activated" banner
  kHideAbilityBanner = 2, ///< Hide the "ability activated" banner
  kShowMessage = 3,

  kRecoverHp = 4,
  kLifestealHeal = 5, ///< Heal by absorbing part of the damage just dealt
  kDealDamage = 6,
  kAdjustHpDirectly = 7,
  ///< Move/average HP without it counting as damage or a heal (e.g. Pain Split)
  kRecoverPp = 8,
  kReducePp = 9,

  kCureStatus = 10,
  kInflictStatus = 11,
  kSetStickyStatusParams = 12,
  ///< Re-apply/overwrite the parameters of a status the target already has

  kAdjustStatStage = 13,
  kSetStatStageDirectly = 14,
  kResetAllStatStages = 15,
  kOverwriteBaseStat = 16,
  ///< Force-overwrite a raw stat number (Attack, Defense...)
  kRemoveStatDebuffs = 17,

  kKnockOut = 18,
  kChangeType = 19,
  kAddExtraType = 20, ///< Grant a 3rd type on top of the existing two

  kSetTurnMarker = 21, ///< Set a flag that clears automatically at end of turn
  kClearTurnMarker = 22, ///< Force-clear a turn-scoped flag
  kSetPersistentMarker = 23, ///< Set a flag that survives across turns
  kClearPersistentMarker = 24,

  kAddTeamEffect = 25,
  ///< Add an effect covering one whole team's side (Light Screen, Tailwind...)
  kRemoveTeamEffect = 26,
  kSetTeamEffectPaused = 27,

  kAddFieldEffect = 28, ///< Add a whole-field effect (Trick Room, Gravity...)
  kRemoveFieldEffect = 29,
  kChangeWeather = 30,
  kAddPositionalEffect = 31, ///< Add an effect tied to one specific field slot

  kChangeAbility = 32,
  kSetHeldItem = 33,
  kCheckItemActivation = 34,
  ///< Check whether a held item's effect should trigger
  kActivateItemEffect = 35,
  kConsumeItem = 36,
  kSwapHeldItems = 37,

  kOverwriteMoveData = 38,
  kSetMoveCounter = 39,
  ///< Set a Pokémon's internal move-tracking counter (combo/retaliation moves)
  kDelayedMoveDamage = 40,
  ///< Schedule damage to land on a later turn (Future Sight-style)

  kLeaveBattle = 41,
  kSwitchInPokemon = 42,
  kBatonTouch = 43, ///< Pass stat stages (and similar) to the incoming Pokémon
  kFlinch = 44,
  kRevive = 45,
  kSetWeight = 46,
  kForceSwitchOut = 47,
  ///< Forcibly remove the Pokémon from the field (Roar/Whirlwind-style)
  kForceActImmediately = 48,
  ///< Insert an action for a Pokémon right now, out of normal turn order
  kInterceptPendingMove = 49,
  ///< Intercept a Pokémon that is about to use a specific move
  kDeferActionToTurnEnd = 50,
  ///< Push a Pokémon's action to resolve last this turn
  kSwapActivePokemon = 51,

  kTransform = 52,
  kBreakIllusion = 53,
  kCheckGravityEffects = 54,
  ///< Run the checks triggered when Gravity activates
  kCancelSemiInvulnerableState = 55,
  ///< Cancel a semi-invulnerable state (Fly, Dig, Dive...)

  kPlayVisualEffectAtPosition = 56,
  ///< Play a visual effect at a chosen field position
  kFadeOutMessageWindow = 57,
  kChangeForm = 58,
  kSetMoveEffectVariant = 59,
  ///< Choose which visual variant of a move's effect to play
  kForcePlayMoveEffect = 60,
  ///< Force a move's visual effect to play regardless of normal conditions
  kFreefall = 61,
  kApplyFriendshipBonus = 62,

  kCount = 63,
};
}