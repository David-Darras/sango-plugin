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
 * @file custom_abilities.cc
 * @brief Example abilities: Toxic Drizzle, the Surge abilities, Beast Boost...
 *
 * Each ability has:
 * 1. one or more reaction functions: what the ability does,
 * 2. a ReactionTable: when the game calls each reaction,
 * 3. one battle::GameExtension::AddAbility() call.
 */

#include "custom_battle.h"

#include "battle/constant/field_effect_kind.h"
#include "battle/constant/moment_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/terrain_kind.h"
#include "battle/constant/weather.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/pokemon.h"
#include "battle/native/situation.h"
#include "battle/patch/game_extension.h"
#include "overworld/patch/weather_override.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/type.h"

namespace battle {
namespace {

// The target of the drizzle abilities. A battle id is (client * 6 + party slot):
// id 12 is the first Pokemon of client 2.
constexpr UID kDrizzleTarget{12};

// Toxic Drizzle: violet acid rain that poisons the opponent on entry.
void ToxicDrizzleReaction(Listener* self, Controller* controller, UID owner,
                          s32* local_state) {
  overworld::WeatherOverride::GetInstance().mode =
      overworld::WeatherMode::kToxic;
  controller->SetWeather(owner, Weather::kRain, ItemId::kNone, true);

  auto* poison = static_cast<InflictStatusMutation*>(
      controller->Create(MutationKind::kInflictStatus, kDrizzleTarget));
  poison->status = StatusCondition::kPoison;
  poison->status_data.raw = 1;
  poison->target_id = kDrizzleTarget;
  controller->Apply(poison);
}

// Radioactive Drizzle: green radioactive rain. It also drops both Pokemon to
// 1 HP, changes the holder into Fan Rotom and changes the opponent to the
// Ground type.
void RadioactiveDrizzleReaction(Listener* self, Controller* controller,
                                UID owner, s32* local_state) {
  overworld::WeatherOverride::GetInstance().mode =
      overworld::WeatherMode::kRadioactive;
  controller->SetWeather(owner, Weather::kRain, ItemId::kNone, true);

  auto* self_pkm = controller->GetPokemon(owner);
  auto* opponent_pkm = controller->GetPokemon(kDrizzleTarget);

  auto* hp = static_cast<AdjustHpDirectlyMutation*>(
      controller->Create(MutationKind::kAdjustHpDirectly, owner));
  hp->target_count = 2;
  hp->target_ids[0] = UID{self_pkm->uid};
  hp->volume[0] = -(self_pkm->hp - 1);
  hp->target_ids[1] = UID{opponent_pkm->uid};
  hp->volume[1] = -(opponent_pkm->hp - 1);
  controller->Apply(hp);

  auto* form = static_cast<ChangeFormMutation*>(
      controller->Create(MutationKind::kChangeForm, owner));
  form->target_id = owner;
  form->form = FormId::kRotomFan;
  controller->Apply(form);

  auto* type = static_cast<ChangeTypeMutation*>(
      controller->Create(MutationKind::kChangeType, owner));
  type->next_type = TypeId::kGround;
  type->target_id = kDrizzleTarget;
  type->suppress_default_message = 0;
  type->show_failure_message_if_unchanged = 0;
  controller->Apply(type);
}

void CastMove(Controller* controller, UID owner, MoveId move) {
  controller->ExecuteMove(controller->GetPokemon(owner), move);
}

// Reality Warp: uses a series of moves that change the battlefield on entry.
void RealityWarpReaction(Listener* self, Controller* controller, UID owner,
                         s32* local_state) {
  if (Situation::Get(SituationKey::kPokemonId) != owner.value) return;

  CastMove(controller, owner, MoveId::kTrickRoom);
  CastMove(controller, owner, MoveId::kWonderRoom);
  CastMove(controller, owner, MoveId::kMagicRoom);
  CastMove(controller, owner, MoveId::kGravity);
  CastMove(controller, owner, MoveId::kGrassyTerrain);
  CastMove(controller, owner, MoveId::kStealthRock);
}

// Sets a terrain when the holder enters the battle. The banner of the ability
// shows.
void SetTerrain(Controller* controller, UID owner, TerrainKind terrain) {
  if (Situation::Get(SituationKey::kPokemonId) != owner.value) return;

  auto* mut = static_cast<AddFieldEffectMutation*>(
      controller->Create(MutationKind::kAddFieldEffect, owner));
  mut->show_ability_banner = true;
  mut->effect = FieldEffectKind::kTerrain;
  mut->terrain = terrain;
  mut->duration.raw = 1;
  mut->message.id = MutationMessageId::kNone;
  controller->Apply(mut);
}

void ElectricSurgeReaction(Listener* self, Controller* controller, UID owner,
                           s32* local_state) {
  SetTerrain(controller, owner, TerrainKind::kElectricTerrain);
}

// Generation VI has no Psychic Terrain: Misty Terrain replaces it.
void PsychicSurgeReaction(Listener* self, Controller* controller, UID owner,
                          s32* local_state) {
  SetTerrain(controller, owner, TerrainKind::kMistyTerrain);
}

void GrassySurgeReaction(Listener* self, Controller* controller, UID owner,
                         s32* local_state) {
  SetTerrain(controller, owner, TerrainKind::kGrassyTerrain);
}

void MistySurgeReaction(Listener* self, Controller* controller, UID owner,
                        s32* local_state) {
  SetTerrain(controller, owner, TerrainKind::kMistyTerrain);
}

// Returns the stat with the highest value (Attack, Defense, Sp. Atk, Sp. Def
// or Speed).
StatStageEffectKind GetBeastBoostStage(Pokemon* pkm) {
  u16 max = pkm->attack;
  StatStageEffectKind kind = StatStageEffectKind::kAttack;
  if (pkm->defense > max) {
    kind = StatStageEffectKind::kDefense;
    max = pkm->defense;
  }
  if (pkm->special_attack > max) {
    kind = StatStageEffectKind::kSpecialAttack;
    max = pkm->special_attack;
  }
  if (pkm->special_defense > max) {
    kind = StatStageEffectKind::kSpecialDefense;
    max = pkm->special_defense;
  }
  if (pkm->speed > max) {
    kind = StatStageEffectKind::kSpeed;
    max = pkm->speed;
  }
  return kind;
}

// Beast Boost: raises the highest stat by one stage for each knocked-out
// target.
void BeastBoostReaction(Listener* self, Controller* controller, UID owner,
                        s32* local_state) {
  if (Situation::Get(SituationKey::kMoveUserId) != owner.value) return;

  s8 count = 0;
  for (u32 i = 0; i < Situation::Get(SituationKey::kTargetCount); i++) {
    const u32 target = static_cast<u32>(SituationKey::kTargetId1) + i;
    UID id{(u8)Situation::Get(static_cast<SituationKey>(target))};
    auto* pkm = controller->GetPokemon(id);
    if (pkm->hp == 0) count++;
  }

  if (count <= 0) return;

  auto* mut = static_cast<AdjustStatStageMutation*>(
      controller->Create(MutationKind::kAdjustStatStage, owner));
  mut->show_ability_banner = true;
  mut->target_count = 1;
  mut->target_ids[0] = owner;
  mut->stage_kind = GetBeastBoostStage(controller->GetPokemon(owner));
  mut->stage_delta = count;
  controller->Apply(mut);
}

const ReactionTable kToxicDrizzleReactions[] = {
    {MomentKind::kPokemonEntered, ToxicDrizzleReaction},
};
const ReactionTable kRadioactiveDrizzleReactions[] = {
    {MomentKind::kPokemonEntered, RadioactiveDrizzleReaction},
};
const ReactionTable kRealityWarpReactions[] = {
    {MomentKind::kPokemonEntered, RealityWarpReaction},
};
const ReactionTable kElectricSurgeReactions[] = {
    {MomentKind::kPokemonEntered, ElectricSurgeReaction},
    {MomentKind::kAfterAbilityChange, ElectricSurgeReaction},
};
const ReactionTable kPsychicSurgeReactions[] = {
    {MomentKind::kPokemonEntered, PsychicSurgeReaction},
    {MomentKind::kAfterAbilityChange, PsychicSurgeReaction},
};
const ReactionTable kGrassySurgeReactions[] = {
    {MomentKind::kPokemonEntered, GrassySurgeReaction},
    {MomentKind::kAfterAbilityChange, GrassySurgeReaction},
};
const ReactionTable kMistySurgeReactions[] = {
    {MomentKind::kPokemonEntered, MistySurgeReaction},
    {MomentKind::kAfterAbilityChange, MistySurgeReaction},
};
const ReactionTable kBeastBoostReactions[] = {
    {MomentKind::kDamageSequenceEndRealHit, BeastBoostReaction},
};

} // namespace

void RegisterCustomAbilities() {
  GameExtension::AddAbility(
      {kAbilityToxicDrizzle, u"Toxic Drizzle",
       u"Summons acid rain that\npoisons all Pokémon on entry.",
       kToxicDrizzleReactions, SIZE(kToxicDrizzleReactions)});
  GameExtension::AddAbility(
      {kAbilityRadioactiveDrizzle, u"Radioactive Drizzle",
       u"Summons a radioactive rain\nthat triggers Imposter on entry.",
       kRadioactiveDrizzleReactions, SIZE(kRadioactiveDrizzleReactions)});
  GameExtension::AddAbility({kAbilityRealityWarp, u"Reality Warp", u"???",
                             kRealityWarpReactions,
                             SIZE(kRealityWarpReactions)});
  GameExtension::AddAbility(
      {kAbilityElectricSurge, u"Electric Surge",
       u"Turns the ground into Electric Terrain\nwhen the Pokémon enters a "
       u"battle.",
       kElectricSurgeReactions, SIZE(kElectricSurgeReactions)});
  GameExtension::AddAbility(
      {kAbilityPsychicSurge, u"Psychic Surge",
       u"Turns the ground into Psychic Terrain\nwhen the Pokémon enters a "
       u"battle.",
       kPsychicSurgeReactions, SIZE(kPsychicSurgeReactions)});
  GameExtension::AddAbility(
      {kAbilityGrassySurge, u"Grassy Surge",
       u"Turns the ground into Grassy Terrain\nwhen the Pokémon enters a "
       u"battle.",
       kGrassySurgeReactions, SIZE(kGrassySurgeReactions)});
  GameExtension::AddAbility(
      {kAbilityMistySurge, u"Misty Surge",
       u"Turns the ground into Misty Terrain\nwhen the Pokémon enters a "
       u"battle.",
       kMistySurgeReactions, SIZE(kMistySurgeReactions)});
  GameExtension::AddAbility(
      {kAbilityBeastBoost, u"Beast Boost",
       u"Boosts the Pokémon's highest stat\nwhen it knocks out a target.",
       kBeastBoostReactions, SIZE(kBeastBoostReactions)});
}

} // namespace battle
