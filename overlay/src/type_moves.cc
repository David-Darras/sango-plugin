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
 * @file type_moves.cc
 * @brief Four moves, one for each new type: Prism Lance (Light), Sonic Requiem
 * (Sound), Ion Storm (Plasma) and Dreamfall (Dream).
 *
 * Each move has its own data, its own battle reactions and its own animation
 * (type_moves_animation.cc).
 */

#include "custom_battle.h"

#include "battle/constant/moment_kind.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/terrain_kind.h"
#include "battle/constant/field_effect_kind.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/pokemon.h"
#include "battle/native/situation.h"
#include "battle/patch/game_extension.h"
#include "pokemon/native/move_data.h"

namespace battle {
namespace {

// The first target of the move. The battle ids are (client * 6 + party slot):
// the player is client 0 and the first opponent is client 2.
UID FindTarget(UID owner) {
  if (Situation::Get(SituationKey::kTargetCount) > 0) {
    return UID{static_cast<u8>(Situation::Get(SituationKey::kTargetId1))};
  }
  return UID{static_cast<u8>(owner.value < 6 ? 12 : 0)};
}

// Changes one stat stage of a Pokémon.
void ChangeStage(Controller* controller, UID owner, UID target,
                 StatStageEffectKind stat, s8 delta) {
  // The reactions run after the damage: a target that fainted keeps its stats.
  const Pokemon* pkm = controller->GetPokemon(target);
  if (pkm == nullptr || pkm->hp == 0) return;

  auto* mut = static_cast<AdjustStatStageMutation*>(
      controller->Create(MutationKind::kAdjustStatStage, owner));
  mut->target_count = 1;
  mut->target_ids[0] = target;
  mut->stage_kind = stat;
  mut->stage_delta = delta;
  mut->from_move = 1;
  controller->Apply(mut);
}

// Prism Lance: a special beam of light. The light blinds the target (its
// accuracy falls by two stages), and the beam never misses.
void PatchPrismLanceData(pokemon::MoveData& move) {
  move.power = 85;
  move.accuracy = 101;
  move.base_pp = 15;
  move.type = kTypeLight;
  move.category = MoveCategory::kDamage;
  move.damage_category = DamageCategory::kSpecial;
  move.crit_stage = 1;
}

void PrismLanceReaction(Listener* self, Controller* controller, UID owner,
                        s32* local_state) {
  ChangeStage(controller, owner, FindTarget(owner),
              StatStageEffectKind::kAccuracy, -2);
}

// Sonic Requiem: a loud special attack that shatters the guard of the target:
// its Defense and its Sp. Def fall, and it can flinch.
void PatchSonicRequiemData(pokemon::MoveData& move) {
  move.power = 100;
  move.accuracy = 90;
  move.base_pp = 10;
  move.type = kTypeSound;
  move.category = MoveCategory::kDamage;
  move.damage_category = DamageCategory::kSpecial;
  move.flinch_rate = 30;
}

void SonicRequiemReaction(Listener* self, Controller* controller, UID owner,
                          s32* local_state) {
  const UID target = FindTarget(owner);
  ChangeStage(controller, owner, target, StatStageEffectKind::kDefense, -1);
  ChangeStage(controller, owner, target, StatStageEffectKind::kSpecialDefense,
              -1);
}

// Ion Storm: a special attack with a high power. It can paralyse, it covers the
// field with Electric Terrain, and the user loses a part of its HP (recoil).
void PatchIonStormData(pokemon::MoveData& move) {
  move.power = 110;
  move.accuracy = 85;
  move.base_pp = 5;
  move.type = kTypePlasma;
  move.category = MoveCategory::kDamageStatus;
  move.damage_category = DamageCategory::kSpecial;
  move.effect_id = StatusCondition::kParalysis;
  move.effect_rate = 40;
  move.effect_turn_type = EffectTurnType::kPermanent;
  move.recoil = -25;
}

void IonStormReaction(Listener* self, Controller* controller, UID owner,
                      s32* local_state) {
  auto* mut = static_cast<AddFieldEffectMutation*>(
      controller->Create(MutationKind::kAddFieldEffect, owner));
  mut->effect = FieldEffectKind::kTerrain;
  mut->terrain = TerrainKind::kElectricTerrain;
  mut->duration.raw = 5;
  mut->message.id = MutationMessageId::kNone;
  controller->Apply(mut);
}

// Dreamfall: a special attack that puts the target to sleep (30 %) and heals
// the user for half of the damage. The reaction adds a part that does not
// depend on the damage: the user recovers a quarter of its HP.
void PatchDreamfallData(pokemon::MoveData& move) {
  move.power = 80;
  move.accuracy = 100;
  move.base_pp = 10;
  move.type = kTypeDream;
  move.category = MoveCategory::kDamageStatus;
  move.damage_category = DamageCategory::kSpecial;
  move.effect_id = StatusCondition::kSleep;
  move.effect_rate = 30;
  move.effect_turn_type = EffectTurnType::kPermanent;
  move.drain = 50;
}

void DreamfallReaction(Listener* self, Controller* controller, UID owner,
                       s32* local_state) {
  Pokemon* user = controller->GetPokemon(owner);
  if (user == nullptr || user->hp == 0 || user->hp == user->max_hp) return;

  auto* mut = static_cast<RecoverHpMutation*>(
      controller->Create(MutationKind::kRecoverHp, owner));
  mut->heal_amount = user->max_hp / 4;
  mut->target_id = owner;
  mut->message.id = MutationMessageId::kNone;
  controller->Apply(mut);
}

const ReactionTable kPrismLanceReactions[] = {
    {MomentKind::kAfterDamagingAllTargets, PrismLanceReaction},
};
const ReactionTable kSonicRequiemReactions[] = {
    {MomentKind::kAfterDamagingAllTargets, SonicRequiemReaction},
};
const ReactionTable kIonStormReactions[] = {
    {MomentKind::kAfterDamagingAllTargets, IonStormReaction},
};
const ReactionTable kDreamfallReactions[] = {
    {MomentKind::kAfterDamagingAllTargets, DreamfallReaction},
};

} // namespace

void RegisterTypeMoves() {
  GameExtension::AddMove(
      {kMovePrismLance, u"Prism Lance",
       u"A beam of light that never misses\nand blinds the target.",
       PatchPrismLanceData, nullptr, kPrismLanceReactions,
       SIZE(kPrismLanceReactions)});
  GameExtension::AddMove(
      {kMoveSonicRequiem, u"Sonic Requiem",
       u"A deafening wave that shatters the\nguard of the target.",
       PatchSonicRequiemData, nullptr, kSonicRequiemReactions,
       SIZE(kSonicRequiemReactions)});
  GameExtension::AddMove(
      {kMoveIonStorm, u"Ion Storm",
       u"A storm of plasma that charges the\nfield. The user takes recoil.",
       PatchIonStormData, nullptr, kIonStormReactions,
       SIZE(kIonStormReactions)});
  GameExtension::AddMove(
      {kMoveDreamfall, u"Dreamfall",
       u"Dream dust that can put the target\nto sleep and heals the user.",
       PatchDreamfallData, nullptr, kDreamfallReactions,
       SIZE(kDreamfallReactions)});
}

} // namespace battle
