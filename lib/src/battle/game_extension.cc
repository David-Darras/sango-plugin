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
 * @file game_extension.cc
 * @brief Adds new moves and new abilities to the game.
 *
 * The declarations are in battle/patch/game_extension.h.
 */

#include "battle/patch/game_extension.h"
#include "battle/patch/move_animation.h"
#include "pokemon/patch/custom_shop.h"

#include "core/constant/archive_id.h"
#include "core/hook.h"
#include "battle/native/broadcaster.h"
#include "battle/native/listener.h"
#include "battle/native/pokemon.h"
#include "battle/constant/priority_tier.h"
#include "pokemon/native/move_data.h"

namespace battle {

namespace {
core::Hook<void(MoveId, String*)> get_move_name_hook;
core::Hook<void(String*, AbilityId)> get_ability_name_hook;
core::Hook<void(String*, AbilityId)> get_ability_description_hook;
core::Hook<void(uptr, u32, u32)> set_ability_name_hook;
core::Hook<void(uptr, u32, u32)> set_move_name_hook;
core::Hook<void(Message*, u32, String*)> message_get_string_hook;
core::Hook<uptr(Pokemon*)> get_battle_ability_handler_hook;
core::Hook<uptr(Pokemon*, MoveId, u32)> get_battle_move_handler_hook;
core::Hook<void(uptr, u32, bool)> battle_load_animation_hook;
core::Hook<void(uptr, u32, u32, u32)> battle_load_effect_hook;
core::Hook<u32(uptr, MoveId)> load_move_data_hook;
} // namespace

// The text files of the game that hold move and ability names. The
// MessageGetStringHook below replaces the text of the new entries.
namespace {
// The game uses two text files for move names and two for item names.
constexpr u32 kMoveNameFile1 = 14;
constexpr u32 kMoveNameFile2 = 15;
constexpr u32 kMoveDescriptionFile = 16;
constexpr u32 kAbilityDescriptionFile = 36;
constexpr u32 kAbilityNameFile = 37;
constexpr u32 kItemNameFile1 = 113;
constexpr u32 kItemNameFile2 = 116;

// The move that lends its data to a new move before patch_data runs.
constexpr u32 kTemplateMove = static_cast<u32>(MoveId::kPound);
} // namespace

// Registration.
bool GameExtension::AddMove(const MoveSpec& spec) {
  auto& self = GetInstance();
  if (self.move_count_ >= kMaxMoves || FindMove(spec.id) != nullptr) {
    return false;
  }
  self.moves_[self.move_count_++] = spec;
  return true;
}

bool GameExtension::AddAbility(const AbilitySpec& spec) {
  auto& self = GetInstance();
  if (self.ability_count_ >= kMaxAbilities ||
      FindAbility(spec.id) != nullptr) {
    return false;
  }
  self.abilities_[self.ability_count_++] = spec;
  return true;
}

const MoveSpec* GameExtension::FindMove(MoveId id) {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.move_count_; i++) {
    if (self.moves_[i].id == id) return &self.moves_[i];
  }
  return nullptr;
}

const AbilitySpec* GameExtension::FindAbility(AbilityId id) {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.ability_count_; i++) {
    if (self.abilities_[i].id == id) return &self.abilities_[i];
  }
  return nullptr;
}

// Hooks.
void GameExtension::Initialize() {
  get_move_name_hook.Install(pokemon::address::kGetMoveName, GetMoveNameHook);
  get_ability_name_hook.Install(pokemon::address::kGetAbilityName,
                                GetAbilityNameHook);
  get_ability_description_hook.Install(pokemon::address::kGetAbilityDescription,
                                       GetAbilityDescriptionHook);
  set_ability_name_hook.Install(pokemon::address::kSetAbilityName,
                                SetAbilityNameHook);
  set_move_name_hook.Install(pokemon::address::kSetMoveName, SetMoveNameHook);
  message_get_string_hook.Install(sys::address::kMessageGetString,
                                  MessageGetStringHook);
  // The battle code is loaded only during a battle: PatchBattleLoad()
  // enables these hooks when a battle starts.
  get_battle_ability_handler_hook.Install(address::kRegisterAbilityListener,
                                          GetBattleAbilityHandlerHook,
                                          address::kVtable);
  get_battle_move_handler_hook.Install(address::kRegisterMoveListener,
                                       GetBattleMoveHandlerHook,
                                       address::kVtable);
  battle_load_animation_hook.Install(address::kLoadAnimation,
                                     BattleLoadAnimationHook, address::kVtable);
  battle_load_effect_hook.Install(address::kLoadEffect, BattleLoadEffectHook,
                                  address::kVtable);
  load_move_data_hook.Install(pokemon::address::kLoadMoveData, LoadMoveData);
}

void GameExtension::BattleLoadEffectHook(uptr self, u32 archive_id,
                                         u32 file_id, u32 type) {
  if (MoveAnimations::AreEffectsOnDemand() &&
      archive_id == static_cast<u32>(core::ArchiveId::kMoveEffectParticle)) {
    return;
  }
  battle_load_effect_hook(self, archive_id, file_id, type);
}

void GameExtension::BattleLoadAnimationHook(uptr self, u32 id, bool is_move) {
  MoveAnimations::LoadEffectsOnDemand(false);
  if (kMoveAnimationCount != 0) is_move = id < kMoveAnimationCount;
  if (is_move) MoveAnimations::Resolve(static_cast<MoveId>(id), &id, &is_move);
  if (is_move) {
    const MoveSpec* spec = FindMove(static_cast<MoveId>(id));
    if (spec != nullptr && spec->patch_animation != nullptr) {
      spec->patch_animation(id, is_move);
    }
  }
  battle_load_animation_hook(self, id, is_move);
}

// The game reads the data of a move into a work buffer: the data pointer is
// at self + 8 and the move id at self + 4. For a new move, the game loads the
// template move, then patch_data edits the copy and the id is set back.
u32 GameExtension::LoadMoveData(uptr self, MoveId move_id) {
  const MoveSpec* spec = FindMove(move_id);

  const MoveId id =
      spec != nullptr ? static_cast<MoveId>(kTemplateMove) : move_id;
  u32 result = load_move_data_hook(self, id);

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
  const MoveSpec* spec = FindMove(move);
  if (spec == nullptr) return false;
  output->Set(spec->name);
  return true;
}

bool GameExtension::PatchMoveDescription(MoveId move, String* output) {
  const MoveSpec* spec = FindMove(move);
  if (spec == nullptr) return false;
  output->Set(spec->description);
  return true;
}

bool GameExtension::PatchAbilityName(AbilityId ability, String* output) {
  const AbilitySpec* spec = FindAbility(ability);
  if (spec == nullptr) return false;
  output->Set(spec->name);
  return true;
}

bool GameExtension::PatchAbilityDescription(AbilityId ability, String* output) {
  const AbilitySpec* spec = FindAbility(ability);
  if (spec == nullptr) return false;
  output->Set(spec->description);
  return true;
}

// The game registers one listener for the ability of each Pokemon in battle.
// For a new ability, the listener gets the reactions of the AbilitySpec.
uptr GameExtension::GetBattleAbilityHandlerHook(Pokemon* pkm) {
  const AbilitySpec* spec = FindAbility(pkm->ability);
  if (spec != nullptr) {
    return (uptr)Broadcaster::Register(
        ListenerSource::kAbility, static_cast<u32>(pkm->ability),
        PriorityTier::kActiveMoveDefault, 1000, UID{pkm->uid},
        const_cast<ReactionTable*>(spec->reactions), spec->reaction_count);
  }
  return get_battle_ability_handler_hook(pkm);
}

// The game registers one listener for the move that a Pokemon uses. For a new
// move, the listener gets the reactions of the MoveSpec.
uptr GameExtension::GetBattleMoveHandlerHook(Pokemon* pkm, MoveId move,
                                             u32 x) {
  const MoveSpec* spec = FindMove(move);
  if (spec != nullptr) {
    return (uptr)Broadcaster::Register(
        ListenerSource::kActiveMove, static_cast<u32>(move),
        PriorityTier::kActiveMoveDefault, x, UID{pkm->uid},
        const_cast<ReactionTable*>(spec->reactions), spec->reaction_count);
  }
  return get_battle_move_handler_hook(pkm, move, x);
}

void GameExtension::MessageGetStringHook(Message* self, u32 str_id,
                                         String* output) {
  message_get_string_hook(self, str_id, output);
  switch (self->file_id) {
    case kMoveNameFile1:
    case kMoveNameFile2:
      PatchMoveName(static_cast<MoveId>(str_id), output);
      break;
    case kMoveDescriptionFile:
      PatchMoveDescription(static_cast<MoveId>(str_id), output);
      break;
    case kAbilityDescriptionFile:
      PatchAbilityDescription(static_cast<AbilityId>(str_id), output);
      break;
    case kAbilityNameFile:
      PatchAbilityName(static_cast<AbilityId>(str_id), output);
      break;
    case kItemNameFile1:
    case kItemNameFile2:
      pokemon::CustomShop::PatchItemName(static_cast<ItemId>(str_id), output);
      break;
    default:
      break;
  }
}

void GameExtension::SetAbilityNameHook(uptr self, u32 archive, u32 ability) {
  set_ability_name_hook(self, archive, ability);
  String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
  PatchAbilityName(static_cast<AbilityId>(ability), output);
}

void GameExtension::SetMoveNameHook(uptr self, u32 archive, u32 move) {
  set_move_name_hook(self, archive, move);
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
