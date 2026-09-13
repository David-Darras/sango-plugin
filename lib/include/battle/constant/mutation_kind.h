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
  kShowMessage = 3, ///< Display a battle message

  kRecoverHp = 4, ///< Heal HP
  kLifestealHeal = 5, ///< Heal by absorbing part of the damage just dealt
  kDealDamage = 6, ///< Deal damage
  kAdjustHpDirectly = 7,
  ///< Move/average HP without it counting as damage or a heal (e.g. Pain Split)
  kRecoverPp = 8, ///< Restore PP
  kReducePp = 9, ///< Reduce PP

  kCureStatus = 10, ///< Cure a status condition
  kInflictStatus = 11, ///< Inflict a status condition

  kAdjustStatStage = 12, ///< Raise or lower a stat stage
  kSetStatStageDirectly = 13, ///< Force a stat stage to an exact value
  kResetAllStatStages = 14, ///< Flatten every stat stage back to neutral
  kOverwriteBaseStat = 15,
  ///< Force-overwrite a raw stat number (Attack, Defense...)
  kRemoveStatDebuffs = 16, ///< Clear only the negative stat stages

  kKnockOut = 17, ///< Make the Pokémon faint
  kChangeType = 18, ///< Change the Pokémon's type
  kAddExtraType = 19, ///< Grant a 3rd type on top of the existing two

  kSetTurnMarker = 20, ///< Set a flag that clears automatically at end of turn
  kClearTurnMarker = 21, ///< Force-clear a turn-scoped flag
  kSetPersistentMarker = 22, ///< Set a flag that survives across turns
  kClearPersistentMarker = 23, ///< Clear a persistent flag

  kAddTeamEffect = 24,
  ///< Add an effect covering one whole team's side (Light Screen, Tailwind...)
  kRemoveTeamEffect = 25, ///< Remove a team-side effect
  kSetTeamEffectPaused = 26, ///< Pause or resume a team-side effect

  kAddFieldEffect = 27, ///< Add a whole-field effect (Trick Room, Gravity...)
  kRemoveFieldEffect = 28, ///< Remove a whole-field effect
  kChangeWeather = 29, ///< Change the current weather
  kAddPositionalEffect = 30, ///< Add an effect tied to one specific field slot

  kChangeAbility = 31, ///< Overwrite the current ability
  kSetHeldItem = 32, ///< Overwrite the held item
  kCheckItemActivation = 33,
  ///< Check whether a held item's effect should trigger
  kActivateItemEffect = 34, ///< Trigger a held item's effect
  kConsumeItem = 35, ///< Consume one's own held item
  kSwapHeldItems = 36, ///< Swap held items between two Pokémon

  kOverwriteMoveData = 37, ///< Overwrite a move's stored data
  kSetMoveCounter = 38,
  ///< Set a Pokémon's internal move-tracking counter (combo/retaliation moves)
  kDelayedMoveDamage = 39,
  ///< Schedule damage to land on a later turn (Future Sight-style)

  kLeaveBattle = 40, ///< Remove the Pokémon from the battle entirely
  kSwitchInPokemon = 41, ///< Bring in a party member
  kBatonTouch = 42, ///< Pass stat stages (and similar) to the incoming Pokémon
  kFlinch = 43, ///< Cause flinching
  kRevive = 44, ///< Bring a fainted Pokémon back
  kSetWeight = 45, ///< Override the Pokémon's weight
  kForceSwitchOut = 46,
  ///< Forcibly remove the Pokémon from the field (Roar/Whirlwind-style)
  kForceActImmediately = 47,
  ///< Insert an action for a Pokémon right now, out of normal turn order
  kInterceptPendingMove = 48,
  ///< Intercept a Pokémon that is about to use a specific move
  kDeferActionToTurnEnd = 49,
  ///< Push a Pokémon's action to resolve last this turn
  kSwapActivePokemon = 50, ///< Swap the field positions of two active Pokémon

  kTransform = 51, ///< Transform into the target
  kBreakIllusion = 52, ///< Reveal a Pokémon disguised by Illusion
  kCheckGravityEffects = 53,
  ///< Run the checks triggered when Gravity activates
  kCancelSemiInvulnerableState = 54,
  ///< Cancel a semi-invulnerable state (Fly, Dig, Dive...)

  kPlayVisualEffectAtPosition = 55,
  ///< Play a visual effect at a chosen field position
  kFadeOutMessageWindow = 56, ///< Fade out the message box
  kChangeForm = 57, ///< Change the Pokémon's form
  kSetMoveEffectVariant = 58,
  ///< Choose which visual variant of a move's effect to play
  kForcePlayMoveEffect = 59,
  ///< Force a move's visual effect to play regardless of normal conditions
  kApplyFriendshipBonus = 60, ///< Apply an effect scaled by friendship level

  kCount = 61, ///< Number of mutation kinds
};
}