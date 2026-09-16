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
struct Mutation {
  MutationKind kind : 8;
  u32 owner_id : 5; ///< The Pokémon this mutation is scoped to
  u32 struct_size : 10;
  ///< Size of the concrete mutation struct, for the pool allocator
  u32 show_ability_banner : 1;
  ///< Show the "ability activated" banner for owner_id
  u32 skip_if_previous_failed : 1;
  ///< Cancelled if the previous mutation in the chain failed
  u32 cancel_if_owner_fainted : 1;
  ///< Cancelled if owner_id has already fainted
  u32 in_use : 1; ///< Internal: this work slot is currently being processed
  u32 _padding : 5;
};

struct UseItemMutation : Mutation {
  u32 skip_if_hp_full : 1; ///< Don't run if the Pokémon is already at full HP
  u32 allow_if_fainted : 1; ///< Runs even if the Pokémon has fainted
  u32 _padding : 30;
};

struct ShowMessageMutation : Mutation {
  MutationMessage message;
};

struct RecoverHpMutation : Mutation {
  u16 heal_amount;
  UID target_id;
  u8 ignore_heal_block; ///< Bypass Heal Block-style checks
  MutationMessage message;
};

struct LifestealHealMutation : Mutation {
  u16 heal_amount;
  UID healed_id;
  UID damaged_id; ///< The Pokémon whose damage is being absorbed from
  MutationMessage message;
};

struct DealDamageMutation : Mutation {
  u16 damage_amount;
  UID target_id;
  u8 ignore_if_semi_invulnerable : 1;
  ///< No effect on a Pokémon vanished via Fly/Dig-style state
  u8 play_visual_effect : 1;
  u8 _padding : 6;
  u16 visual_effect_id;
  FieldPosition effect_start_position; ///< FieldPosition::kNone if unused
  FieldPosition effect_end_position; ///< FieldPosition::kNone if unused
  MutationMessage message;
};

struct AdjustHpDirectlyMutation : Mutation {
  u8 target_count;
  u8 suppress_gauge_effect;
  u8 suppress_item_reaction;
  UID target_ids[10];
  int volume[10];
};

// --- kRecoverPp / kReducePp (shared struct) ------------------------------
struct PpAdjustmentMutation : Mutation {
  u8 amount;
  UID target_id;
  u8 move_slot_index;
  u8 affect_temporary_move_slot : 1;
  ///< Targets a temporary move-work slot, not the real moveset
  u8 allow_if_fainted : 1; ///< Usable on a fainted Pokémon (trainer item use)
  u8 _padding : 6;
  MutationMessage message;
};

struct CureStatusMutation : Mutation {
  StatusCondition status; ///< Extendable status code
  UID target_ids[12];
  u8 target_count;
  u8 suppress_default_message;
  MutationMessage message;
};

struct InflictStatusMutation : Mutation {
  StatusCondition status;
  StatusData status_data;
  u8 show_failure_message;
  ///< Show the failure message even if a special cause blocked it
  u8 suppress_default_message;
  u8 suppress_item_reaction;
  UID target_id;
  StatusOverwriteMode overwrite_mode;
  MutationMessage message;
  ///< Custom message (suppress_default_message should be set alongside)
};

struct AdjustStatStageMutation : Mutation {
  StatStageEffectKind stage_kind;
  u32 effect_serial; ///< 0 if unused
  UID target_ids[10];
  u8 target_count;
  s8 stage_delta; ///< 0 forces a reset instead
  u8 suppress_default_message : 1;
  u8 show_failure_message : 1;
  u8 from_move : 1;
  u8 show_message_before_animation : 1;
  u8 always_pierce_substitute : 1;
  u8 run_substitute_pierce_check : 1;
  MutationMessage message;
};

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

struct RemoveStatDebuffsMutation : Mutation {
  UID target_id;
};

struct ResetAllStatStagesMutation : Mutation {
  u8 target_count;
  UID target_ids[10];
};

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

struct KnockOutMutation : Mutation {
  UID target_id;
  u8 allow_if_already_fainted;
  ///< Forces the fainting presentation even if already down
  MoveId recorded_move_id; ///< Records this as caused by a move, if valid
  MutationMessage message;
};

struct ChangeTypeMutation : Mutation {
  TypeId next_type;
  UID target_id;
  u8 suppress_default_message;
  u8 show_failure_message_if_unchanged;
  ///< Show a failure message if the new type is identical to the current one
};

struct AddExtraTypeMutation : Mutation {
  TypeId ex_type;
  UID target_id;
};

// --- kSetTurnMarker / kClearTurnMarker (shared struct) ---------------------
struct TurnMarkerMutation : Mutation {
  TurnMarker marker;
  UID target_id;
};

// --- kSetPersistentMarker / kClearPersistentMarker (shared struct) ---------
struct PersistentMarkerMutation : Mutation {
  PersistentMarker marker;
  UID target_id;
};

struct AddTeamEffectMutation : Mutation {
  TeamEffectKind effect;
  StatusData duration;
  FieldSide side;
  MutationMessage message;
};

struct RemoveTeamEffectMutation : Mutation {
  u8 flags[4];
  FieldSide side;
};

struct SetTeamEffectPausedMutation : Mutation {
  u8 flags[4];
  FieldSide side;
  u8 resume; ///< true = resume instead of pause
};

struct AddFieldEffectMutation : Mutation {
  FieldEffectKind effect;
  TerrainKind terrain; ///< Only read when effect == kTerrain
  StatusData duration;
  MutationMessage message;
  u8 register_as_dependent_on_failure;
  ///< If adding fails, still track owner_id as depending on it
};

struct RemoveFieldEffectMutation : Mutation {
  FieldEffectKind effect;
};

struct ChangeWeatherMutation : Mutation {
  Weather weather;
  u8 turns;
  u8 cleared_by_air_lock;
  MutationMessage message;
};

struct AddPositionalEffectMutation : Mutation {
  PositionalEffectKind effect;
  FieldPosition position;
  int params[4];
  u8 param_count;
};

struct ChangeAbilityMutation : Mutation {
  u16 ability_id; ///< kNoAbility clears the ability entirely
  UID target_id;
  u8 affects_others_with_same_ability;
  u8 skip_next_member_in_event;
  ///< Skip one entry-event pass (avoids Trace-style loops)
  MutationMessage message;
};

struct SetHeldItemMutation : Mutation {
  ItemId item_id; ///< ItemId::kNone clears the held item
  UID target_id;
  u8 clear_own_consumption_record;
  u8 clear_other_consumption_record;
  UID consumption_record_target_id;
  u8 call_consumed_event_if_cleared;
  MutationMessage message;
};

struct SwapHeldItemsMutation : Mutation {
  UID target_id; ///< Swaps with owner_id from the header
  MutationMessage message;
  MutationMessage sub_message_1;
  MutationMessage sub_message_2;
};

struct CheckItemActivationMutation : Mutation {
  UID target_id;
  ItemReactionKind reaction_kind;
};

struct ActivateItemEffectMutation : Mutation {
  UID target_id;
  u8 treat_as_eaten_berry; ///< For Bug Bite/Pluck-style effects
  ItemId item_id;
};

struct ConsumeItemMutation : Mutation {
  u8 skip_action;
  u8 skip_berry_eaten_flag;
  MutationMessage message;
};

struct OverwriteMoveDataMutation : Mutation {
  UID target_id;
  u8 move_slot_index;
  u8 pp_max; ///< 0 = default
  u8 persists_after_battle;
  MoveId move_id;
};

struct SetMoveCounterMutation : Mutation {
  UID target_id;
  u8 counter_id;
  u8 value;
};

struct DelayedMoveDamageMutation : Mutation {
  UID attacker_id;
  UID target_id;
  FieldPosition attacker_position;
  MoveId move_id;
};

struct SwitchInPokemonMutation : Mutation {
  MutationMessage pre_message; ///< Shown as the switch begins
  MutationMessage message; ///< Shown on success
  UID target_id;
  u8 forbid_interrupt; ///< Blocks pursuit-style interrupts
};

struct BatonTouchMutation : Mutation {
  UID source_id;
  UID target_id;
};

struct FlinchMutation : Mutation {
  UID target_id;
  u8 chance_percent;
};

struct ReviveMutation : Mutation {
  UID target_id;
  u16 heal_amount;
  MutationMessage message;
};

struct SetWeightMutation : Mutation {
  UID target_id;
  u16 weight_value;
  MutationMessage message;
};

struct ForceSwitchOutMutation : Mutation {
  u16 visual_effect_id;
  UID target_id;
  u8 force_switch_mode : 4;
  u8 ignore_level_check : 4;
  MutationMessage message;
};

struct ForceActImmediatelyMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

struct InterceptPendingMoveMutation : Mutation {
  MoveId move_id;
};

struct DeferActionToTurnEndMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

struct SwapActivePokemonMutation : Mutation {
  UID first_id;
  UID second_id;
  MutationMessage message;
};

struct TransformMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

struct BreakIllusionMutation : Mutation {
  UID target_id;
  MutationMessage message;
};

struct CancelSemiInvulnerableStateMutation : Mutation {
  UID target_id;
  PersistentMarker state_to_cancel;
  MutationMessage message;
};

struct PlayVisualEffectAtPositionMutation : Mutation {
  u16 visual_effect_id;
  FieldPosition start_position; ///< FieldPosition::kNone if unused
  FieldPosition end_position; ///< FieldPosition::kNone if unused
  u16 reserved_queue_slot;
  u8 reserve_queue_slot;
  u8 fade_out_message_window;
  MutationMessage message; ///< Shown after the effect plays
};

struct ChangeFormMutation : Mutation {
  UID target_id;
  FormId form;
  MutationMessage message;
};

struct SetMoveEffectVariantMutation : Mutation {
  u8 variant_index;
};

// --- kForcePlayMoveEffect ------------------------------------------------
// No extra data beyond Mutation.

struct ApplyFriendshipBonusMutation : Mutation {
  UID target_id;
  FriendshipEffect effect_kind;
  MutationMessage message;
};
}