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
#include "common.h"
#include "feature/core/hook_manager.h"
#include "game/battle/manager.h"
#include "game/constant/ability.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/move_type.h"
#include "game/constant/status_condition.h"
#include "game/constant/weather.h"
#include "game/global_data/move.h"

// Plugin-defined extensions to the game's ability / move ID spaces. They are
// not part of the vanilla tables in game/constant, so they are declared here
// as typed constants rather than added to the enums themselves.
constexpr Ability kAbilityRandom = static_cast<Ability>(255);
constexpr MoveID kMoveAbsoluteZero = static_cast<MoveID>(863);
constexpr MoveID kMoveSolarFlare = static_cast<MoveID>(864);

namespace global_data {
struct Move;
}

namespace feature {
class GameExtension {
  MAKE_SINGLETON(GameExtension)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGetMoveName,
                            ADDRESS_GET_MOVE_NAME,
                            (uptr)GetMoveNameHook);
    HookManager::Initialize(HookID::kGetAbilityName,
                            ADDRESS_GET_ABILITY_NAME,
                            (uptr)GetAbilityNameHook);
    HookManager::Initialize(HookID::kGetAbilityDescription,
                            ADDRESS_GET_ABILITY_DESCRIPTION,
                            (uptr)GetAbilityDescriptionHook);
    HookManager::Initialize(HookID::kSetAbilityName,
                            ADDRESS_SET_ABILITY_NAME,
                            (uptr)SetAbilityNameHook);
    HookManager::Initialize(HookID::kSetMoveName,
                            ADDRESS_SET_MOVE_NAME,
                            (uptr)SetMoveNameHook);
    HookManager::Initialize(HookID::kMessageGetString,
                            ADDRESS_MESSAGE_GET_STRING,
                            (uptr)MessageGetStringHook);
    HookManager::Initialize(HookID::kBattleGetAbilityHandler,
                            ADDRESS_BATTLE_GET_ABILITY_HANDLER,
                            (uptr)GetBattleAbilityHandlerHook,
                            false);
    HookManager::Initialize(HookID::kBattleGetMoveHandler,
                            ADDRESS_BATTLE_GET_MOVE_HANDLER,
                            (uptr)GetBattleMoveHandlerHook,
                            false);
    HookManager::Initialize(HookID::kBattleLoadAnimation,
                            ADDRESS_BATTLE_LOAD_ANIMATION,
                            (uptr)BattleLoadAnimationHook,
                            false);
    HookManager::Initialize(HookID::kLoadMoveData,
                            ADDRESS_LOAD_MOVE_DATA,
                            (uptr)LoadMoveData);
  }

  // `id` is polymorphic here (a move id only when `is_move` is set,
  // something else otherwise), so it stays a raw u32.
  static void BattleLoadAnimationHook(uptr self, u32 id, bool is_move) {
    if (is_move) {
      if (id == static_cast<u32>(kMoveAbsoluteZero)) {
        id = static_cast<u32>(MoveID::kIcePunch);
      }
      if (id == static_cast<u32>(kMoveSolarFlare)) {
        id = static_cast<u32>(MoveID::kFirePunch);
      }
    }
    HookManager::Call<void>(HookID::kBattleLoadAnimation, self, id, is_move);
  }

  // Hooked game function: `move_id` arrives in a register as a raw u16,
  // which is exactly how `MoveID` is passed, so the ABI is unchanged.
  static u32 LoadMoveData(uptr self, MoveID move_id) {
    u32 id = static_cast<u16>(move_id);
    if (move_id == kMoveAbsoluteZero || move_id == kMoveSolarFlare)
      id = 1;
    u32 result = HookManager::Call<u32>(HookID::kLoadMoveData, self, id);
    if (move_id == kMoveAbsoluteZero) {
      auto& move = *(global_data::Move*)(READ32(self + 8));
      move.power = 0;
      move.accuracy = 100;
      move.base_pp = 5;
      move.type = MoveType::kIce;
      move.effect_id = static_cast<u16>(StatusCondition::kFreeze);
      move.effect_rate = 100;
      move.effect_turn_type = 1;
      move.flinch_rate = 100;
      move.category = 4;
      move.damage_category = 1;
      WRITE16(self + 4, static_cast<u16>(move_id));
    }
    if (move_id == kMoveSolarFlare) {
      auto& move = *(global_data::Move*)(READ32(self + 8));
      move.power = 0;
      move.accuracy = 100;
      move.base_pp = 5;
      move.type = MoveType::kFire;
      move.effect_id = static_cast<u16>(StatusCondition::kBurn);
      move.effect_rate = 100;
      move.effect_turn_type = 1;
      move.flinch_rate = 100;
      move.category = 4;
      move.damage_category = 1;
      WRITE16(self + 4, static_cast<u16>(move_id));
    }
    return result;
  }

  static bool PatchMoveName(MoveID move, String* output) {
    switch (move) {
      case kMoveAbsoluteZero:
        output->Set(u"Absolute Zero");
        return true;
      case kMoveSolarFlare:
        output->Set(u"Solar Flare");
        return true;
      default:
        return false;
    }
  }

  static bool PatchMoveDescription(MoveID move, String* output) {
    switch (move) {
      case kMoveAbsoluteZero:
        output->Set(
            u"Instantly freezes the target solid.");
        return true;
      case kMoveSolarFlare:
        output->Set(
            u"Summons blinding sunlight\nand instantly leaves the target with a severe burn.");
        return true;
      default:
        return false;
    }
  }

  static bool PatchAbilityName(Ability ability, String* output) {
    switch (ability) {
      case kAbilityRandom:
        output->Set(u"Random");
        return true;
      default:
        return false;
    }
  }

  static bool PatchAbilityDescription(Ability ability, String* output) {
    switch (ability) {
      case kAbilityRandom:
        output->Set(u"???");
        return true;
      default:
        return false;
    }
  }

  STATIC_INLINE void PatchBattleLoad() {
    HookManager::ForceEnable(HookID::kBattleGetAbilityHandler);
    HookManager::ForceEnable(HookID::kBattleGetMoveHandler);
    HookManager::ForceEnable(HookID::kBattleLoadAnimation);
  }

  static u32 GetValue(u32 var) {
    return ((u32(*)(u32))ADDRESS_BATTLE_GET_VARIABLE)(var);
  }

  static void UpdateWeather(uptr manager, u8 uid, BattleWeather weather,
                            ItemID item, bool infinite) {
    ((void(*)(uptr, u8, BattleWeather, ItemID, bool))
      ADDRESS_BATTLE_UPDATE_WEATHER)(manager, uid, weather, item, infinite);
  }

  static void handler_random(uptr a, uptr manager, u32 uid, uptr c) {
    static u8 weather = 0;
    weather++;
    if (weather == 6) weather = 0;
    UpdateWeather(manager, uid, static_cast<BattleWeather>(1 + weather),
                  ItemID::kNone, true);

    // imposter
    // ((void(*)(uptr, uptr, u32, uptr))0x7B9568)(a, manager, uid, c);
  }

  static void handler_sun(uptr a, uptr manager, u32 uid, uptr c) {
    UpdateWeather(manager, uid, BattleWeather::kHarshSunlight, ItemID::kNone,
                  true);
  }

  static uptr GetBattleAbilityHandlerHook(battle::Pokemon* pkm) {
    static struct {
      u32 event;
      uptr handler;
    } table[] = {
        {94, (uptr)handler_random}
    };
    if (pkm->ability == kAbilityRandom) {
      // Same game entry point as GetBattleMoveHandlerHook below, but the id
      // slot is dispatched by the first argument: 4 selects the ability
      // handler table, so type this call site's slot as `Ability`.
      return ((uptr(*)(u32, Ability, u32, u32, u32, u32, u32))
        ADDRESS_BATTLE_GET_EVENT_HANDLER)(
          4, pkm->ability, 0, 1000, pkm->uid, (uptr)table,
          SIZE(table));
    }
    return HookManager::Call<uptr>(HookID::kBattleGetAbilityHandler, pkm);
  }

  static uptr GetBattleMoveHandlerHook(battle::Pokemon* pkm, MoveID move,
                                       u32 x) {
    ui::LogApplication::Print(u"Move Handler %u", static_cast<u16>(move));
    static struct {
      u32 event;
      uptr handler;
    } table[] = {
        {39, (uptr)handler_sun}
    };
    if (move == kMoveSolarFlare || move == kMoveAbsoluteZero) {
      // Same address as GetBattleAbilityHandlerHook above: 0 selects the move
      // handler table, so this call site's id slot is a `MoveID`.
      return ((uptr(*)(u32, MoveID, u32, u32, u32, u32, u32))
        ADDRESS_BATTLE_GET_EVENT_HANDLER)(
          0, move, 0, x, pkm->uid, (uptr)table,
          SIZE(table));
    }
    return HookManager::Call<uptr>(HookID::kBattleGetMoveHandler, pkm, move, x);
  }

  static void MessageGetStringHook(Message* self, u32 str_id, String* output) {
    HookManager::Call<void>(HookID::kMessageGetString, self, str_id, output);
    switch (self->file_id) {
      case 14:
      case 15:
        PatchMoveName(static_cast<MoveID>(str_id), output);
        break;
      case 16:
        PatchMoveDescription(static_cast<MoveID>(str_id), output);
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

  // `archive` is a message-archive slot index, not an id; `ability` /
  // `move` arrive as full 32-bit words, wider than Ability/MoveID, so
  // both stay raw and are narrowed at the comparison instead.
  static void SetAbilityNameHook(uptr self, u32 archive, u32 ability) {
    HookManager::Call<void>(HookID::kSetAbilityName, self, archive, ability);
    String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
    PatchAbilityName(static_cast<Ability>(ability), output);
  }

  static void SetMoveNameHook(uptr self, u32 archive, u32 move) {
    HookManager::Call<void>(HookID::kSetMoveName, self, archive, move);
    String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
    PatchMoveName(static_cast<MoveID>(move), output);
  }

  static void GetAbilityNameHook(String* output, Ability ability) {
    if (PatchAbilityName(ability, output) || ability >= Ability::kCount)
      return;

    GetAbilityName().GetString(static_cast<u8>(ability), output);
  }

  static void GetMoveNameHook(MoveID move, String* output) {
    if (PatchMoveName(move, output) || move >= MoveID::kCount)
      return;

    GetMoveName().GetString(static_cast<u16>(move), output);
  }

  static void GetAbilityDescriptionHook(String* output, Ability ability) {
    if (PatchAbilityDescription(ability, output)
        || ability >= Ability::kCount)
      return;

    GetAbilityDescription().GetString(static_cast<u8>(ability), output);
  }

  STATIC_INLINE Message& GetAbilityName() {
    return *(Message*)READ32(ADDRESS_GLOBAL_DATA_ABILITY_NAME_MESSAGE);
  }

  STATIC_INLINE Message& GetAbilityDescription() {
    return *(Message*)READ32(ADDRESS_GLOBAL_DATA_ABILITY_DESCRIPTION_MESSAGE);
  }

  STATIC_INLINE Message& GetMoveName() {
    return *(Message*)READ32(ADDRESS_GLOBAL_DATA_MOVE_NAME_MESSAGE);
  }
};
}