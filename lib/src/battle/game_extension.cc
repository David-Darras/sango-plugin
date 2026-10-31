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
#include "battle/patch/type_chart.h"
#include "battle/patch/move_animation.h"
#include "pokemon/patch/custom_shop.h"

#include "core/constant/archive_id.h"
#include "core/hook.h"
#include "core/patch/text_editor.h"
#include "battle/native/broadcaster.h"
#include "battle/native/listener.h"
#include "battle/native/pokemon.h"
#include "battle/constant/priority_tier.h"
#include "pokemon/native/move_data.h"
#include "battle/address.h"
#include "pokemon/address.h"
#include "system/address.h"

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
constexpr u32 kTypeNameFile = 18;
constexpr u32 kGameTypeCount = 18;
constexpr u32 kItemNameFile1 = 113;
constexpr u32 kItemNameFile2 = 116;

// The move that lends its data to a new move before patch_data runs.
constexpr u32 kTemplateMove = static_cast<u32>(MoveId::kPound);

// The texts of the battle are built in memory, and the file number is 0 (the
// same as the texts of the scripts). The texts "<Pokemon> used <move>!" have
// three texts for each move (the Pokemon of the player, a wild Pokemon, a
// Pokemon of the opponent): the text number is the move number times 3, plus
// 0, 1 or 2.
constexpr u32 kBattleAttackFile = 0;
constexpr u32 kAttackTextCount = 3;

// Replaces the first `from` in the text by `to`. The text can hold codes equal
// to 0 (the parameters of a tag): the length of the text is its `size`.
void ReplaceText(String* text, const c16* from, const c16* to) {
  const c16* source = text->GetBuffer();
  const u32 length_in = text->size;
  u32 from_length = 0;
  while (from[from_length] != 0) from_length++;
  u32 to_length = 0;
  while (to[to_length] != 0) to_length++;
  if (from_length == 0 || from_length > length_in) return;

  for (u32 start = 0; start + from_length <= length_in; start++) {
    u32 matched = 0;
    while (matched < from_length && source[start + matched] == from[matched]) {
      matched++;
    }
    if (matched != from_length) continue;

    const u32 length_out = length_in - from_length + to_length;
    if (length_out + 1 > text->capacity) return;
    c16 result[128];
    if (length_out + 1 > SIZE(result)) return;
    u32 length = 0;
    for (u32 i = 0; i < start; i++) result[length++] = source[i];
    for (u32 i = 0; i < to_length; i++) result[length++] = to[i];
    for (u32 i = start + from_length; i < length_in; i++) {
      result[length++] = source[i];
    }
    result[length] = 0;
    for (u32 i = 0; i <= length; i++) text->buffer[i] = result[i];
    text->size = static_cast<u16>(length);
    return;
  }
}
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

bool GameExtension::AddType(const TypeSpec& spec) {
  auto& self = GetInstance();
  const u32 id = static_cast<u32>(spec.id);
  if (id == kGameTypeCount || id >= kMaxTypes || self.is_type_used_[id]) {
    return false;
  }
  self.types_[id] = spec;
  self.is_type_used_[id] = true;
  return true;
}

bool GameExtension::GetTypeIconPath(const TypeSpec& spec, c16* path,
                                    u32 capacity) {
  static const c16 kFolder[] = u"sdmc:/sango/types/";
  static const c16 kExtension[] = u".bclim";
  if (spec.name == nullptr) return false;
  u32 length = 0;
  for (const c16* c = kFolder; *c != 0; c++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *c;
  }
  for (const c16* c = spec.name; *c != 0; c++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *c;
  }
  for (const c16* c = kExtension; *c != 0; c++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *c;
  }
  path[length] = 0;
  return true;
}

const TypeSpec* GameExtension::FindType(u32 id) {
  auto& self = GetInstance();
  return id < kMaxTypes && self.is_type_used_[id] ? &self.types_[id] : nullptr;
}

u32 GameExtension::GetTypeListSize() {
  auto& self = GetInstance();
  u32 size = kGameTypeCount + 1;
  for (u32 i = kGameTypeCount + 1; i < kMaxTypes; i++) {
    if (self.is_type_used_[i]) size = i + 1;
  }
  return size;
}

void GameExtension::SetTypeChart(const TypeMultiplier* table, u32 type_count) {
  TypeChart::SetTable(table, type_count);
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
  // A new move has an id after the last animation of the game: the game
  // would not take it for a move.
  const bool is_new_move =
      is_move && FindMove(static_cast<MoveId>(id)) != nullptr;
  if (kMoveAnimationCount != 0) is_move = id < kMoveAnimationCount;
  if (is_move || is_new_move) {
    is_move = true;
    MoveAnimations::Resolve(static_cast<MoveId>(id), &id, &is_move);
  }
  if (is_move) {
    const MoveSpec* spec = FindMove(static_cast<MoveId>(id));
    if (spec != nullptr && spec->patch_animation != nullptr) {
      spec->patch_animation(id, is_move);
    } else if (spec != nullptr) {
      // A new move without an animation: the game has no animation for its
      // number, so it plays the animation of a simple move.
      id = static_cast<u32>(MoveId::kTackle);
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

// The name of a type: a new type has no text in the file of the game.
bool GameExtension::PatchTypeName(u32 type, String* output) {
  const TypeSpec* spec = FindType(type);
  if (spec == nullptr || spec->name == nullptr) return false;
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
  // The text file of the types has 18 texts: a new type has its name here,
  // and the game does not read its text.
  if (self->file_id == kTypeNameFile && str_id >= kGameTypeCount &&
      PatchTypeName(str_id, output)) {
    core::TextEditor::OnString(self, str_id, output);
    return;
  }
  // A new move has no text "<Pokemon> used <move>!" in the file of the game:
  // the plugin takes the text of a simple move and changes the name.
  if (self->file_id == kBattleAttackFile) {
    const MoveSpec* spec =
        FindMove(static_cast<MoveId>(str_id / kAttackTextCount));
    if (spec != nullptr) {
      message_get_string_hook(
          self, kTemplateMove * kAttackTextCount + str_id % kAttackTextCount,
          output);
      c16 template_name[48] = {};
      String name;
      name.buffer = template_name;
      name.capacity = SIZE(template_name);
      name.size = 0;
      GetMoveName().GetString(kTemplateMove, &name);
      ReplaceText(output, template_name, spec->name);
      core::TextEditor::OnString(self, str_id, output);
      return;
    }
  }
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
    case kTypeNameFile:
      PatchTypeName(str_id, output);
      break;
    case kItemNameFile1:
    case kItemNameFile2:
      pokemon::CustomShop::PatchItemName(static_cast<ItemId>(str_id), output);
      break;
    default:
      break;
  }
  // The last step: the text editor sees the text with the patches above, and
  // replaces it when the player changed it.
  core::TextEditor::OnString(self, str_id, output);
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
