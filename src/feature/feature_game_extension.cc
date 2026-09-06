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

#include "feature/battle/feature_game_extension.h"

#include "feature/core/hook_manager.h"
#include "feature/weather_manager.h"
#include "game/battle/broadcaster.h"
#include "game/battle/controller.h"
#include "game/battle/mutation.h"
#include "game/battle/pokemon.h"
#include "game/battle/situation.h"
#include "game/constant/item.h"
#include "game/constant/move_type.h"
#include "game/battle/constant/priority_tier.h"
#include "game/constant/status_condition.h"
#include "game/constant/weather.h"
#include "game/global_data/move.h"

namespace feature {
// ----------------------------------------------------------------------
// Ability reactions.
// ----------------------------------------------------------------------

// Toxic Drizzle: violet acid rain that poisons all Pokemon on entry.
static void ToxicDrizzleReaction(battle::Listener* self,
                                 battle::Controller* controller,
                                 battle::UID owner, s32* local_state) {
  WeatherManager::GetInstance().mode = WeatherMode::kToxic;
  controller->SetWeather(owner, BattleWeather::kRain, ItemId::kNone, true);

  auto* poison = static_cast<battle::InflictStatusMutation*>(
    controller->Create(battle::MutationKind::kInflictStatus,
                       battle::UID{12}));
  poison->status = StatusId::kPoison;
  poison->status_data.raw = 1;
  poison->target_id = battle::UID{12};
  controller->Apply(poison);
}

// Radioactive Drizzle: green radioactive rain. Also drops both Pokemon to
// 1 HP, turns the ability holder into Rotom Fan, and forces the opponent to
// Ground type - kept as-is from earlier experimentation.
static void RadioactiveDrizzleReaction(battle::Listener* self,
                                       battle::Controller* controller,
                                       battle::UID owner,
                                       s32* local_state) {
  WeatherManager::GetInstance().mode = WeatherMode::kRadioactive;
  controller->SetWeather(owner, BattleWeather::kRain, ItemId::kNone, true);

  auto* self_pkm = controller->GetPokemon(owner);
  auto* opponent_pkm = controller->GetPokemon(battle::UID{12});

  auto* hp = static_cast<battle::AdjustHpDirectlyMutation*>(
    controller->Create(battle::MutationKind::kAdjustHpDirectly, owner));
  hp->target_count = 2;
  hp->target_ids[0] = battle::UID{self_pkm->uid};
  hp->volume[0] = -(self_pkm->hp - 1);
  hp->target_ids[1] = battle::UID{opponent_pkm->uid};
  hp->volume[1] = -(opponent_pkm->hp - 1);
  controller->Apply(hp);

  auto* form = static_cast<battle::ChangeFormMutation*>(
    controller->Create(battle::MutationKind::kChangeForm, owner));
  form->target_id = owner;
  form->form_number = static_cast<u8>(Form::kRotomFan);
  controller->Apply(form);

  auto* type = static_cast<battle::ChangeTypeMutation*>(
    controller->Create(battle::MutationKind::kChangeType, owner));
  type->next_type = PokemonType::kGround;
  type->target_id = battle::UID{12};
  type->suppress_default_message = 0;
  type->show_failure_message_if_unchanged = 0;
  controller->Apply(type);

  // imposter
  // ((void(*)(uptr, uptr, u32, uptr))0x7B9568)(a, controller, owner.value, local_state);
}

static void CastMove(battle::Controller* controller, battle::UID owner,
                     MoveId move) {
  controller->ExecuteMove(controller->GetPokemon(owner), move);
}

// Reality Warp: casts a barrage of field-warping moves on entry.
static void RealityWarpReaction(battle::Listener* self,
                                battle::Controller* controller,
                                battle::UID owner, s32* local_state) {
  s32 current_pokemon =
      battle::Situation::Get(battle::SituationKey::kPokemonId);
  if (current_pokemon != owner.value) return;

  CastMove(controller, owner, MoveId::kTrickRoom);
  CastMove(controller, owner, MoveId::kWonderRoom);
  CastMove(controller, owner, MoveId::kMagicRoom);
  CastMove(controller, owner, MoveId::kGravity);
  CastMove(controller, owner, MoveId::kGrassyTerrain);
  CastMove(controller, owner, MoveId::kStealthRock);
}

static const battle::ReactionTable kToxicDrizzleReactions[] = {
    {battle::MomentKind::kPokemonEntered, ToxicDrizzleReaction},
};
static const battle::ReactionTable kRadioactiveDrizzleReactions[] = {
    {battle::MomentKind::kPokemonEntered, RadioactiveDrizzleReaction},
};
static const battle::ReactionTable kRealityWarpReactions[] = {
    {battle::MomentKind::kPokemonEntered, RealityWarpReaction},
};

// ----------------------------------------------------------------------
// Ability table. To add a new custom ability: pick an unused id below
// kAbilityRealityWarp, write its reaction(s) above, and add one entry here -
// name/description/dispatch are all handled generically from this table.
// ----------------------------------------------------------------------
static const AbilitySpec kAbilities[] = {
    {kAbilityToxicDrizzle, u"Toxic Drizzle",
     u"Summons acid rain that\npoisons all Pokémon on entry.",
     kToxicDrizzleReactions, SIZE(kToxicDrizzleReactions)},
    {kAbilityRadioactiveDrizzle, u"Radioactive Drizzle",
     u"Summons a radioactive rain\nthat triggers Imposter on entry.",
     kRadioactiveDrizzleReactions, SIZE(kRadioactiveDrizzleReactions)},
    {kAbilityRealityWarp, u"Reality Warp", u"???",
     kRealityWarpReactions, SIZE(kRealityWarpReactions)},
};

// ----------------------------------------------------------------------
// Move reactions and data patches.
// ----------------------------------------------------------------------
static void AbsoluteZeroHailReaction(battle::Listener* self,
                                     battle::Controller* controller,
                                     battle::UID owner, s32* local_state) {
  controller->SetWeather(owner, BattleWeather::kHail, ItemId::kNone, true);
}

static void SolarFlareSunReaction(battle::Listener* self,
                                  battle::Controller* controller,
                                  battle::UID owner, s32* local_state) {
  controller->SetWeather(owner, BattleWeather::kHarshSunlight, ItemId::kNone,
                         true);
}

static void PatchAbsoluteZeroData(global_data::Move& move) {
  move.power = 0;
  move.accuracy = 100;
  move.base_pp = 50;
  move.type = MoveType::kIce;
  move.effect_id = static_cast<u16>(StatusCondition::kFreeze);
  move.effect_rate = 100;
  move.effect_turn_type = 1;
  move.flinch_rate = 100;
  move.category = 4;
  move.damage_category = 1;
}

static void PatchSolarFlareData(global_data::Move& move) {
  move.power = 0;
  move.accuracy = 100;
  move.base_pp = 50;
  move.type = MoveType::kFire;
  move.effect_id = static_cast<u16>(StatusCondition::kBurn);
  move.effect_rate = 100;
  move.effect_turn_type = 1;
  move.flinch_rate = 100;
  move.category = 4;
  move.damage_category = 1;
}

static void UseHailAnimation(u32& id, bool& is_move) {
  id = 19;
  is_move = false;
}

static void UseSunAnimation(u32& id, bool& is_move) {
  id = 18;
  is_move = false;
}

static const battle::ReactionTable kAbsoluteZeroReactions[] = {
    {battle::MomentKind::kMoveExecutionStart, AbsoluteZeroHailReaction},
};
static const battle::ReactionTable kSolarFlareReactions[] = {
    {battle::MomentKind::kMoveExecutionStart, SolarFlareSunReaction},
};

// ----------------------------------------------------------------------
// Move table. To add a new custom move: pick an unused id, write its
// reaction(s)/data patch/animation above, and add one entry here.
// ----------------------------------------------------------------------
static const MoveSpec kMoves[] = {
    {kMoveAbsoluteZero, u"Absolute Zero",
     u"Summons a hailstorm\nand instantly freezes the target solid.",
     PatchAbsoluteZeroData, UseHailAnimation,
     kAbsoluteZeroReactions, SIZE(kAbsoluteZeroReactions)},
    {kMoveSolarFlare, u"Solar Flare",
     u"Summons blinding sunlight\nand instantly leaves the target with a severe burn.",
     PatchSolarFlareData, UseSunAnimation,
     kSolarFlareReactions, SIZE(kSolarFlareReactions)},
};

// ----------------------------------------------------------------------
// GameExtension.
// ----------------------------------------------------------------------
void GameExtension::Initialize() {
  HookManager::Initialize(HookID::kGetMoveName, ADDRESS_GET_MOVE_NAME,
                          (uptr)GetMoveNameHook);
  HookManager::Initialize(HookID::kGetAbilityName, ADDRESS_GET_ABILITY_NAME,
                          (uptr)GetAbilityNameHook);
  HookManager::Initialize(HookID::kGetAbilityDescription,
                          ADDRESS_GET_ABILITY_DESCRIPTION,
                          (uptr)GetAbilityDescriptionHook);
  HookManager::Initialize(HookID::kSetAbilityName, ADDRESS_SET_ABILITY_NAME,
                          (uptr)SetAbilityNameHook);
  HookManager::Initialize(HookID::kSetMoveName, ADDRESS_SET_MOVE_NAME,
                          (uptr)SetMoveNameHook);
  HookManager::Initialize(HookID::kMessageGetString, ADDRESS_MESSAGE_GET_STRING,
                          (uptr)MessageGetStringHook);
  HookManager::Initialize(HookID::kBattleGetAbilityHandler,
                          ADDRESS_BATTLE_GET_ABILITY_HANDLER,
                          (uptr)GetBattleAbilityHandlerHook, false);
  HookManager::Initialize(HookID::kBattleGetMoveHandler,
                          ADDRESS_BATTLE_GET_MOVE_HANDLER,
                          (uptr)GetBattleMoveHandlerHook, false);
  HookManager::Initialize(HookID::kBattleLoadAnimation,
                          ADDRESS_BATTLE_LOAD_ANIMATION,
                          (uptr)BattleLoadAnimationHook, false);
  HookManager::Initialize(HookID::kBattleAddTerrain, 0x00718B10,
                          (uptr)BattleAddTerrainHook, false);
  HookManager::Initialize(HookID::kLoadMoveData, ADDRESS_LOAD_MOVE_DATA,
                          (uptr)LoadMoveData);
}

void GameExtension::PatchBattleLoad() {
  HookManager::ForceEnable(HookID::kBattleGetAbilityHandler);
  HookManager::ForceEnable(HookID::kBattleGetMoveHandler);
  HookManager::ForceEnable(HookID::kBattleLoadAnimation);
  HookManager::ForceEnable(HookID::kBattleAddTerrain);
}

u32 GameExtension::BattleAddTerrainHook(u32 a, u32 b) {
  u32 res = HookManager::Call<u32>(HookID::kBattleAddTerrain, a, b);
  ui::LogApplication::Print(u"res=%u", res);
  return res;
}

void GameExtension::BattleLoadAnimationHook(uptr self, u32 id, bool is_move) {
  if (is_move) {
    for (auto& spec : kMoves) {
      if (id == static_cast<u32>(spec.id) && spec.patch_animation != nullptr) {
        spec.patch_animation(id, is_move);
        break;
      }
    }
  }
  HookManager::Call<void>(HookID::kBattleLoadAnimation, self, id, is_move);
}

u32 GameExtension::LoadMoveData(uptr self, MoveId move_id) {
  const MoveSpec* spec = nullptr;
  for (auto& candidate : kMoves) {
    if (candidate.id == move_id) {
      spec = &candidate;
      break;
    }
  }

  u32 id = static_cast<u16>(move_id);
  if (spec != nullptr) id = 1;

  u32 result = HookManager::Call<u32>(HookID::kLoadMoveData, self, id);

  if (spec != nullptr) {
    if (spec->patch_data != nullptr) {
      auto& move = *(global_data::Move*)(READ32(self + 8));
      spec->patch_data(move);
    }
    WRITE16(self + 4, static_cast<u16>(move_id));
  }
  return result;
}

bool GameExtension::PatchMoveName(MoveId move, String* output) {
  for (auto& spec : kMoves) {
    if (spec.id == move) {
      output->Set(spec.name);
      return true;
    }
  }
  return false;
}

bool GameExtension::PatchMoveDescription(MoveId move, String* output) {
  for (auto& spec : kMoves) {
    if (spec.id == move) {
      output->Set(spec.description);
      return true;
    }
  }
  return false;
}

bool GameExtension::PatchAbilityName(Ability ability, String* output) {
  for (auto& spec : kAbilities) {
    if (spec.id == ability) {
      output->Set(spec.name);
      return true;
    }
  }
  return false;
}

bool GameExtension::PatchAbilityDescription(Ability ability, String* output) {
  for (auto& spec : kAbilities) {
    if (spec.id == ability) {
      output->Set(spec.description);
      return true;
    }
  }
  return false;
}

uptr GameExtension::GetBattleAbilityHandlerHook(battle::Pokemon* pkm) {
  for (auto& spec : kAbilities) {
    if (pkm->ability == spec.id) {
      return (uptr)battle::Broadcaster::Register(
          battle::ListenerSource::kAbility, static_cast<u32>(pkm->ability),
          battle::PriorityTier::kActiveMoveDefault, 1000, battle::UID{pkm->uid},
          const_cast<battle::ReactionTable*>(spec.reactions),
          spec.reaction_count);
    }
  }
  return HookManager::Call<uptr>(HookID::kBattleGetAbilityHandler, pkm);
}

uptr GameExtension::GetBattleMoveHandlerHook(battle::Pokemon* pkm, MoveId move,
                                             u32 x) {
  for (auto& spec : kMoves) {
    if (move == spec.id) {
      return (uptr)battle::Broadcaster::Register(
          battle::ListenerSource::kActiveMove, static_cast<u32>(move),
          battle::PriorityTier::kActiveMoveDefault, x, battle::UID{pkm->uid},
          const_cast<battle::ReactionTable*>(spec.reactions),
          spec.reaction_count);
    }
  }
  return HookManager::Call<uptr>(HookID::kBattleGetMoveHandler, pkm, move, x);
}

void GameExtension::MessageGetStringHook(Message* self, u32 str_id,
                                         String* output) {
  HookManager::Call<void>(HookID::kMessageGetString, self, str_id, output);
  switch (self->file_id) {
    case 14:
    case 15:
      PatchMoveName(static_cast<MoveId>(str_id), output);
      break;
    case 16:
      PatchMoveDescription(static_cast<MoveId>(str_id), output);
      break;
    case 36:
      PatchAbilityDescription(static_cast<Ability>(str_id), output);
      break;
    case 37:
      PatchAbilityName(static_cast<Ability>(str_id), output);
      break;
    default:
      break;
  }
}

void GameExtension::SetAbilityNameHook(uptr self, u32 archive, u32 ability) {
  HookManager::Call<void>(HookID::kSetAbilityName, self, archive, ability);
  String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
  PatchAbilityName(static_cast<Ability>(ability), output);
}

void GameExtension::SetMoveNameHook(uptr self, u32 archive, u32 move) {
  HookManager::Call<void>(HookID::kSetMoveName, self, archive, move);
  String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
  PatchMoveName(static_cast<MoveId>(move), output);
}

void GameExtension::GetAbilityNameHook(String* output, Ability ability) {
  if (PatchAbilityName(ability, output) || ability >= Ability::kCount) return;
  GetAbilityName().GetString(static_cast<u8>(ability), output);
}

void GameExtension::GetMoveNameHook(MoveId move, String* output) {
  if (PatchMoveName(move, output) || move >= MoveId::kCount) return;
  GetMoveName().GetString(static_cast<u16>(move), output);
}

void GameExtension::GetAbilityDescriptionHook(String* output,
                                              Ability ability) {
  if (PatchAbilityDescription(ability, output) || ability >= Ability::kCount)
    return;
  GetAbilityDescription().GetString(static_cast<u8>(ability), output);
}
} // namespace feature