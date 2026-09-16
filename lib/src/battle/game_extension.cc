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

#include "battle/patch/game_extension.h"
#include "pokemon/patch/custom_shop.h"
#include "ui/log_application.h"

#include "core/hook_manager.h"
#include "overworld/patch/weather_override.h"
#include "battle/native/broadcaster.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/pokemon.h"
#include "battle/native/situation.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/type.h"
#include "battle/constant/priority_tier.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/weather.h"
#include "pokemon/native/move_data.h"

namespace battle {
// ----------------------------------------------------------------------
// Ability reactions.
// ----------------------------------------------------------------------

// Toxic Drizzle: violet acid rain that poisons all Pokemon on entry.
static void ToxicDrizzleReaction(Listener* self,
                                 Controller* controller,
                                 UID owner, s32* local_state) {
  overworld::WeatherOverride::GetInstance().mode =
      overworld::WeatherMode::kToxic;
  controller->SetWeather(owner, Weather::kRain, ItemId::kNone, true);

  auto* poison = static_cast<InflictStatusMutation*>(
    controller->Create(MutationKind::kInflictStatus,
                       UID{12}));
  poison->status = StatusCondition::kPoison;
  poison->status_data.raw = 1;
  poison->target_id = UID{12};
  controller->Apply(poison);
}

// Radioactive Drizzle: green radioactive rain. Also drops both Pokemon to
// 1 HP, turns the ability holder into Rotom Fan, and forces the opponent to
// Ground type - kept as-is from earlier experimentation.
static void RadioactiveDrizzleReaction(Listener* self,
                                       Controller* controller,
                                       UID owner,
                                       s32* local_state) {
  overworld::WeatherOverride::GetInstance().mode =
      overworld::WeatherMode::kRadioactive;
  controller->SetWeather(owner, Weather::kRain, ItemId::kNone, true);

  auto* self_pkm = controller->GetPokemon(owner);
  auto* opponent_pkm = controller->GetPokemon(UID{12});

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
  type->target_id = UID{12};
  type->suppress_default_message = 0;
  type->show_failure_message_if_unchanged = 0;
  controller->Apply(type);

  // imposter
  // ((void(*)(uptr, uptr, u32, uptr))0x7B9568)(a, controller, owner.value, local_state);
}

static void CastMove(Controller* controller, UID owner,
                     MoveId move) {
  controller->ExecuteMove(controller->GetPokemon(owner), move);
}

// Reality Warp: casts a barrage of field-warping moves on entry.
static void RealityWarpReaction(Listener* self,
                                Controller* controller,
                                UID owner, s32* local_state) {
  s32 current_pokemon =
      Situation::Get(SituationKey::kPokemonId);
  if (current_pokemon != owner.value) return;

  CastMove(controller, owner, MoveId::kTrickRoom);
  CastMove(controller, owner, MoveId::kWonderRoom);
  CastMove(controller, owner, MoveId::kMagicRoom);
  CastMove(controller, owner, MoveId::kGravity);
  CastMove(controller, owner, MoveId::kGrassyTerrain);
  CastMove(controller, owner, MoveId::kStealthRock);
}

static const ReactionTable kToxicDrizzleReactions[] = {
    {MomentKind::kPokemonEntered, ToxicDrizzleReaction},
};
static const ReactionTable kRadioactiveDrizzleReactions[] = {
    {MomentKind::kPokemonEntered, RadioactiveDrizzleReaction},
};
static const ReactionTable kRealityWarpReactions[] = {
    {MomentKind::kPokemonEntered, RealityWarpReaction},
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
static void AbsoluteZeroHailReaction(Listener* self,
                                     Controller* controller,
                                     UID owner, s32* local_state) {
  controller->SetWeather(owner, Weather::kHail, ItemId::kNone, true);
}

static void SolarFlareSunReaction(Listener* self,
                                  Controller* controller,
                                  UID owner, s32* local_state) {
  controller->SetWeather(owner, Weather::kHarshSunlight, ItemId::kNone,
                         true);
}

static void PatchAbsoluteZeroData(pokemon::MoveData& move) {
  move.power = 0;
  move.accuracy = 100;
  move.base_pp = 50;
  move.type = TypeId::kIce;
  move.effect_id = StatusCondition::kFreeze;
  move.effect_rate = 100;
  move.effect_turn_type = 1;
  move.flinch_rate = 100;
  move.category = 4;
  move.damage_category = 1;
}

static void PatchSolarFlareData(pokemon::MoveData& move) {
  move.power = 0;
  move.accuracy = 100;
  move.base_pp = 50;
  move.type = TypeId::kFire;
  move.effect_id = StatusCondition::kBurn;
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

static const ReactionTable kAbsoluteZeroReactions[] = {
    {MomentKind::kMoveExecutionStart, AbsoluteZeroHailReaction},
};
static const ReactionTable kSolarFlareReactions[] = {
    {MomentKind::kMoveExecutionStart, SolarFlareSunReaction},
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
  core::HookManager::Initialize(HookId::kGetMoveName, pokemon::address::kGetMoveName,
                          (uptr)GetMoveNameHook);
  core::HookManager::Initialize(HookId::kGetAbilityName, pokemon::address::kGetAbilityName,
                          (uptr)GetAbilityNameHook);
  core::HookManager::Initialize(HookId::kGetAbilityDescription,
                          pokemon::address::kGetAbilityDescription,
                          (uptr)GetAbilityDescriptionHook);
  core::HookManager::Initialize(HookId::kSetAbilityName, pokemon::address::kSetAbilityName,
                          (uptr)SetAbilityNameHook);
  core::HookManager::Initialize(HookId::kSetMoveName, pokemon::address::kSetMoveName,
                          (uptr)SetMoveNameHook);
  core::HookManager::Initialize(HookId::kMessageGetString, sys::address::kMessageGetString,
                          (uptr)MessageGetStringHook);
  core::HookManager::Initialize(HookId::kBattleRegisterAbilityListener,
                          address::kRegisterAbilityListener,
                          (uptr)GetBattleAbilityHandlerHook, false);
  core::HookManager::Initialize(HookId::kBattleRegisterMoveListener,
                          address::kRegisterMoveListener,
                          (uptr)GetBattleMoveHandlerHook, false);
  core::HookManager::Initialize(HookId::kBattleLoadAnimation,
                          address::kLoadAnimation,
                          (uptr)BattleLoadAnimationHook, false);
  core::HookManager::Initialize(HookId::kBattleAddTerrain, address::kAddTerrain,
                          (uptr)BattleAddTerrainHook, false);
  core::HookManager::Initialize(HookId::kLoadMoveData, pokemon::address::kLoadMoveData,
                          (uptr)LoadMoveData);
}

void GameExtension::PatchBattleLoad() {
  core::HookManager::ForceEnable(HookId::kBattleRegisterAbilityListener);
  core::HookManager::ForceEnable(HookId::kBattleRegisterMoveListener);
  core::HookManager::ForceEnable(HookId::kBattleLoadAnimation);
  core::HookManager::ForceEnable(HookId::kBattleAddTerrain);
}

u32 GameExtension::BattleAddTerrainHook(u32 a, u32 b) {
  u32 res = core::HookManager::Call<u32>(HookId::kBattleAddTerrain, a, b);
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
  core::HookManager::Call<void>(HookId::kBattleLoadAnimation, self, id, is_move);
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

  u32 result = core::HookManager::Call<u32>(HookId::kLoadMoveData, self, id);

  if (spec != nullptr) {
    if (spec->patch_data != nullptr) {
      auto& move = *(pokemon::MoveData*)(READ32(self + 8));
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

bool GameExtension::PatchAbilityName(AbilityId ability, String* output) {
  for (auto& spec : kAbilities) {
    if (spec.id == ability) {
      output->Set(spec.name);
      return true;
    }
  }
  return false;
}

bool GameExtension::PatchAbilityDescription(AbilityId ability, String* output) {
  for (auto& spec : kAbilities) {
    if (spec.id == ability) {
      output->Set(spec.description);
      return true;
    }
  }
  return false;
}

uptr GameExtension::GetBattleAbilityHandlerHook(Pokemon* pkm) {
  for (auto& spec : kAbilities) {
    if (pkm->ability == spec.id) {
      return (uptr)Broadcaster::Register(
          ListenerSource::kAbility, static_cast<u32>(pkm->ability),
          PriorityTier::kActiveMoveDefault, 1000, UID{pkm->uid},
          const_cast<ReactionTable*>(spec.reactions),
          spec.reaction_count);
    }
  }
  return core::HookManager::Call<uptr>(HookId::kBattleRegisterAbilityListener, pkm);
}

uptr GameExtension::GetBattleMoveHandlerHook(Pokemon* pkm, MoveId move,
                                             u32 x) {
  for (auto& spec : kMoves) {
    if (move == spec.id) {
      return (uptr)Broadcaster::Register(
          ListenerSource::kActiveMove, static_cast<u32>(move),
          PriorityTier::kActiveMoveDefault, x, UID{pkm->uid},
          const_cast<ReactionTable*>(spec.reactions),
          spec.reaction_count);
    }
  }
  return core::HookManager::Call<uptr>(HookId::kBattleRegisterMoveListener, pkm, move, x);
}

void GameExtension::MessageGetStringHook(Message* self, u32 str_id,
                                         String* output) {
  core::HookManager::Call<void>(HookId::kMessageGetString, self, str_id, output);
  switch (self->file_id) {
    case 14:
    case 15:
      PatchMoveName(static_cast<MoveId>(str_id), output);
      break;
    case 16:
      PatchMoveDescription(static_cast<MoveId>(str_id), output);
      break;
    case 36:
      PatchAbilityDescription(static_cast<AbilityId>(str_id), output);
      break;
    case 37:
      PatchAbilityName(static_cast<AbilityId>(str_id), output);
      break;
    case 113: // itemname_wordset, used to build the shop sentences
    case 116: // itemname
      pokemon::CustomShop::PatchItemName(static_cast<ItemId>(str_id), output);
      break;
    default:
      break;
  }
}

void GameExtension::SetAbilityNameHook(uptr self, u32 archive, u32 ability) {
  core::HookManager::Call<void>(HookId::kSetAbilityName, self, archive, ability);
  String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
  PatchAbilityName(static_cast<AbilityId>(ability), output);
}

void GameExtension::SetMoveNameHook(uptr self, u32 archive, u32 move) {
  core::HookManager::Call<void>(HookId::kSetMoveName, self, archive, move);
  String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
  PatchMoveName(static_cast<MoveId>(move), output);
}

void GameExtension::GetAbilityNameHook(String* output, AbilityId ability) {
  if (PatchAbilityName(ability, output) || ability >= AbilityId::kCount) return;
  GetAbilityName().GetString(static_cast<u8>(ability), output);
}

void GameExtension::GetMoveNameHook(MoveId move, String* output) {
  if (PatchMoveName(move, output) || move >= MoveId::kCount) return;
  GetMoveName().GetString(static_cast<u16>(move), output);
}

void GameExtension::GetAbilityDescriptionHook(String* output,
                                              AbilityId ability) {
  if (PatchAbilityDescription(ability, output) || ability >= AbilityId::kCount)
    return;
  GetAbilityDescription().GetString(static_cast<u8>(ability), output);
}

} // namespace battle
