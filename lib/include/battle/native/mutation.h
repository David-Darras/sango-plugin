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
 * @file mutation.h
 * @brief The mutations: the changes that a reaction asks the battle engine for.
 *
 * Call Controller::Create(), cast the result to the structure of the kind,
 * fill it, then call Controller::Apply().
 */

#pragma once
#include "battle/constant/field_effect_kind.h"
#include "battle/constant/field_position.h"
#include "battle/constant/field_side.h"
#include "battle/constant/friendship_effect.h"
#include "battle/constant/item_reaction_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/persistent_marker.h"
#include "battle/constant/positional_effect_kind.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/status_data.h"
#include "battle/constant/status_overwrite_mode.h"
#include "battle/constant/team_effect_kind.h"
#include "battle/constant/terrain_kind.h"
#include "battle/constant/turn_marker.h"
#include "battle/constant/weather.h"
#include "battle/native/mutation_message.h"
#include "battle/native/uid.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/type.h"

namespace battle {
/// The header of all the mutations.
struct Mutation {
  MutationKind kind : 8;
  u32 owner_id : 5; ///< The Pokémon of the mutation.
  u32 struct_size : 10; ///< The size of the mutation structure.
  u32 show_ability_banner : 1; ///< Shows the ability banner of owner_id.
  /// Cancels the mutation when the previous mutation failed.
  u32 skip_if_previous_failed : 1;
  /// Cancels the mutation when owner_id fainted.
  u32 cancel_if_owner_fainted : 1;
  u32 in_use : 1; ///< Used by the engine: the slot is in use.
  u32 _padding : 5;
};

/// The data of MutationKind::kUseItem.
struct UseItemMutation : Mutation {
  u32 skip_if_hp_full : 1; ///< Does nothing when the Pokémon has all its HP.
  u32 allow_if_fainted : 1; ///< Runs also when the Pokémon fainted.
  u32 _padding : 30;
};

/// The data of MutationKind::kShowMessage.
struct ShowMessageMutation : Mutation {
  MutationMessage message;
};

/// The data of MutationKind::kRecoverHp.
struct RecoverHpMutation : Mutation {
  u16 heal_amount;
  UID target_id;
  u8 ignore_heal_block; ///< Ignores Heal Block.
  MutationMessage message;
};

/// The data of MutationKind::kLifestealHeal.
struct LifestealHealMutation : Mutation {
  u16 heal_amount;
  UID healed_id;
  UID damaged_id; ///< The Pokémon that loses the HP.
  MutationMessage message;
};

/// The data of MutationKind::kDealDamage.
struct DealDamageMutation : Mutation {
  u16 damage_amount;
  UID target_id;
  /// No effect on a Pokémon in the sky or underground.
  u8 ignore_if_semi_invulnerable : 1;
  u8 play_visual_effect : 1;
  u8 _padding : 6;
  u16 visual_effect_id;
  FieldPosition effect_start_position; ///< FieldPosition::kNone when not used.
  FieldPosition effect_end_position; ///< FieldPosition::kNone when not used.
  MutationMessage message;
};

/// The data of MutationKind::kAdjustHpDirectly.
struct AdjustHpDirectlyMutation : Mutation {
  u8 target_count;
  u8 suppress_gauge_effect;
  u8 suppress_item_reaction;
  UID target_ids[10];
  int volume[10];
};

/// The data of MutationKind::kRecoverPp and MutationKind::kReducePp.
struct PpAdjustmentMutation : Mutation {
  u8 amount;
  UID target_id;
  u8 move_slot_index;
  /// Changes a temporary move slot, not the real moves.
  u8 affect_temporary_move_slot : 1;
  /// Works on a fainted Pokémon (an item of a trainer).
  u8 allow_if_fainted : 1;
  u8 _padding : 6;
  MutationMessage message;
};

/// The data of MutationKind::kCureStatus.
struct CureStatusMutation : Mutation {
  StatusCondition status; ///< The status condition.
  UID target_ids[12];
  u8 target_count;
  u8 suppress_default_message;
  MutationMessage message;
};

/// The data of MutationKind::kInflictStatus.
struct InflictStatusMutation : Mutation {
  StatusCondition status;
  StatusData status_data;
  /// Shows the failure message also when a special cause blocked it.
  u8 show_failure_message;
  u8 suppress_default_message;
  u8 suppress_item_reaction;
  UID target_id;
  StatusOverwriteMode overwrite_mode;
  /// A custom message. Also set suppress_default_message.
  MutationMessage message;
};

/// The data of MutationKind::kAdjustStatStage.
struct AdjustStatStageMutation : Mutation {
  StatStageEffectKind stage_kind;
  u32 effect_serial; ///< 0 when not used.
  UID target_ids[10];
  u8 target_count;
  s8 stage_delta; ///< 0 resets the stage.
  u8 suppress_default_message : 1;
  u8 show_failure_message : 1;
  u8 from_move : 1;
  u8 show_message_before_animation : 1;
  u8 always_pierce_substitute : 1;
  u8 run_substitute_pierce_check : 1;
  MutationMessage message;
};

/// The data of MutationKind::kSetStatStageDirectly.
struct SetStatStageDirectlyMutation : Mutation {
  UID target_id;
  s8 attack;
  s8 defense;
  s8 special_attack;
  s8 special_defense;
  s8 speed;
  s8 accuracy;
  s8 evasion;
  u8 critical_hit_stage;
};

/// The data of MutationKind::kRemoveStatDebuffs.
struct RemoveStatDebuffsMutation : Mutation {
  UID target_id;
};

/// The data of MutationKind::kResetAllStatStages.
struct ResetAllStatStagesMutation : Mutation {
  u8 target_count;
  UID target_ids[10];
};

/// The data of MutationKind::kOverwriteBaseStat.
struct OverwriteBaseStatMutation : Mutation {
  u16 attack;
  u16 defense;
  u16 special_attack;
  u16 special_defense;
  u16 speed;
  UID target_id;
  u8 overwrite_attack : 1;
  u8 overwrite_defense : 1;
  u8 overwrite_special_attack : 1;
  u8 overwrite_special_defense : 1;
  u8 overwrite_speed : 1;
  u8 _padding : 3;
  MutationMessage message;
};

/// The data of MutationKind::kKnockOut.
struct KnockOutMutation : Mutation {
  UID target_id;
  /// Shows the faint also when the Pokémon has no HP.
  u8 allow_if_already_fainted;
  MoveId recorded_move_id; ///< The move that caused the knock out, if any.
  MutationMessage message;
};

/// The data of MutationKind::kChangeType.
struct ChangeTypeMutation : Mutation {
  TypeId next_type;
  UID target_id;
  u8 suppress_default_message;
  /// Shows a failure message when the type does not change.
  u8 show_failure_message_if_unchanged;
};

/// The data of MutationKind::kAddExtraType.
struct AddExtraTypeMutation : Mutation {
  TypeId ex_type;
  UID target_id;
};

/// The data of MutationKind::kSetTurnMarker and MutationKind::kClearTurnMarker.
struct TurnMarkerMutation : Mutation {
  TurnMarker marker;
  UID target_id;
};

/// The data of MutationKind::kSetPersistentMarker and kClearPersistentMarker.
struct PersistentMarkerMutation : Mutation {
  PersistentMarker marker;
  UID target_id;
};

/// The data of MutationKind::kAddTeamEffect.
struct AddTeamEffectMutation : Mutation {
  TeamEffectKind effect;
  StatusData duration;
  FieldSide side;
  MutationMessage message;
};

/// The data of MutationKind::kRemoveTeamEffect.
struct RemoveTeamEffectMutation : Mutation {
  u8 flags[4];
  FieldSide side;
};

/// The data of MutationKind::kSetTeamEffectPaused.
struct SetTeamEffectPausedMutation : Mutation {
  u8 flags[4];
  FieldSide side;
  u8 resume; ///< true: resumes instead of pauses.
};

/// The data of MutationKind::kAddFieldEffect.
struct AddFieldEffectMutation : Mutation {
  FieldEffectKind effect;
  TerrainKind terrain; ///< Used only when effect is kTerrain.
  StatusData duration;
  MutationMessage message;
  /// When the effect is not added, keeps owner_id linked to it.
  u8 register_as_dependent_on_failure;
};

/// The data of MutationKind::kRemoveFieldEffect.
struct RemoveFieldEffectMutation : Mutation {
  FieldEffectKind effect;
};

/// The data of MutationKind::kChangeWeather.
struct ChangeWeatherMutation : Mutation {
  Weather weather;
  u8 turns;
  u8 cleared_by_air_lock;
  MutationMessage message;
};

/// The data of MutationKind::kAddPositionalEffect.
struct AddPositionalEffectMutation : Mutation {
  PositionalEffectKind effect;
  FieldPosition position;
  int params[4];
  u8 param_count;
};

/// The data of MutationKind::kChangeAbility.
struct ChangeAbilityMutation : Mutation {
  u16 ability_id; ///< 0 removes the ability.
  UID target_id;
  u8 affects_others_with_same_ability;
  /// Skips one entry check (prevents loops like Trace).
  u8 skip_next_member_in_event;
  MutationMessage message;
};

/// The data of MutationKind::kSetHeldItem.
struct SetHeldItemMutation : Mutation {
  ItemId item_id; ///< ItemId::kNone removes the held item.
  UID target_id;
  u8 clear_own_consumption_record;
  u8 clear_other_consumption_record;
  UID consumption_record_target_id;
  u8 call_consumed_event_if_cleared;
  MutationMessage message;
};

/// The data of MutationKind::kSwapHeldItems.
struct SwapHeldItemsMutation : Mutation {
  UID target_id; ///< Exchanges its item with owner_id.
  MutationMessage message;
  MutationMessage sub_message_1;
  MutationMessage sub_message_2;
};

/// The data of MutationKind::kCheckItemActivation.
struct CheckItemActivationMutation : Mutation {
  UID target_id;
  ItemReactionKind reaction_kind;
};

/// The data of MutationKind::kActivateItemEffect.
struct ActivateItemEffectMutation : Mutation {
  UID target_id;
  u8 treat_as_eaten_berry; ///< For Bug Bite and Pluck.
  ItemId item_id;
};

/// The data of MutationKind::kConsumeItem.
struct ConsumeItemMutation : Mutation {
  u8 skip_action;
  u8 skip_berry_eaten_flag;
  MutationMessage message;
};

/// The data of MutationKind::kOverwriteMoveData.
struct OverwriteMoveDataMutation : Mutation {
  UID target_id;
  u8 move_slot_index;
  u8 pp_max; ///< 0: the default value.
  u8 persists_after_battle;
  MoveId move_id;
};

/// The data of MutationKind::kSetMoveCounter.
struct SetMoveCounterMutation : Mutation {
  UID target_id;
  u8 counter_id;
  u8 value;
};

/// The data of MutationKind::kDelayedMoveDamage.
struct DelayedMoveDamageMutation : Mutation {
  UID attacker_id;
  UID target_id;
  FieldPosition attacker_position;
  MoveId move_id;
};

/// The data of MutationKind::kSwitchInPokemon.
struct SwitchInPokemonMutation : Mutation {
  MutationMessage pre_message; ///< The message at the start of the switch.
  MutationMessage message; ///< The message when it succeeds.
  UID target_id;
  u8 forbid_interrupt; ///< Blocks the interruptions like Pursuit.
};

/// The data of MutationKind::kBatonTouch.
struct BatonTouchMutation : Mutation {
  UID source_id;
  UID target_id;
};

/// The data of MutationKind::kFlinch.
struct FlinchMutation : Mutation {
  UID target_id;
  u8 chance_percent;
};

/// The data of MutationKind::kRevive.
struct ReviveMutation : Mutation {
  UID target_id;
  u16 heal_amount;
  MutationMessage message;
};

/// The data of MutationKind::kSetWeight.
struct SetWeightMutation : Mutation {
  UID target_id;
  u16 weight_value;
  MutationMessage message;
};

/// The data of MutationKind::kForceSwitchOut.
struct ForceSwitchOutMutation : Mutation {
  u16 visual_effect_id;
  UID target_id;
  u8 force_switch_mode : 4;
  u8 ignore_level_check : 4;
  MutationMessage message;
};

/// The data of MutationKind::kForceActImmediately.
struct ForceActImmediatelyMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

/// The data of MutationKind::kInterceptPendingMove.
struct InterceptPendingMoveMutation : Mutation {
  MoveId move_id;
};

/// The data of MutationKind::kDeferActionToTurnEnd.
struct DeferActionToTurnEndMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

/// The data of MutationKind::kSwapActivePokemon.
struct SwapActivePokemonMutation : Mutation {
  UID first_id;
  UID second_id;
  MutationMessage message;
};

/// The data of MutationKind::kTransform.
struct TransformMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

/// The data of MutationKind::kBreakIllusion.
struct BreakIllusionMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

/// The data of MutationKind::kCancelSemiInvulnerableState.
struct CancelSemiInvulnerableStateMutation : Mutation {
  UID target_id;
  PersistentMarker state_to_cancel;
  MutationMessage message;
};

/// The data of MutationKind::kPlayVisualEffectAtPosition.
struct PlayVisualEffectAtPositionMutation : Mutation {
  u16 visual_effect_id;
  FieldPosition start_position; ///< FieldPosition::kNone when not used.
  FieldPosition end_position; ///< FieldPosition::kNone when not used.
  u16 reserved_queue_slot;
  u8 reserve_queue_slot;
  u8 fade_out_message_window;
  MutationMessage message; ///< The message after the effect.
};

/// The data of MutationKind::kChangeForm.
struct ChangeFormMutation : Mutation {
  UID target_id;
  FormId form;
  MutationMessage message;
};

/// The data of MutationKind::kSetMoveEffectVariant.
struct SetMoveEffectVariantMutation : Mutation {
  u8 variant_index;
};

/// MutationKind::kForcePlayMoveEffect has no extra data.

/// The data of MutationKind::kApplyFriendshipBonus.
struct ApplyFriendshipBonusMutation : Mutation {
  UID target_id;
  FriendshipEffect effect_kind;
  MutationMessage message;
};
}