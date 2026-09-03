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
#include "hook_manager.h"
#include "game/battle/manager.h"
#include "game/constant/ability.h"
#include "game/constant/move_type.h"
#include "game/constant/status_condition.h"
#include "game/constant/weather.h"
#include "game/global_data/move.h"

#define ABILITY_RANDOM (255)
#define MOVE_ABSOLUTE_ZERO (863)
#define MOVE_SOLAR_FLARE (864)

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
                            0x0076063C,
                            (uptr)GetBattleAbilityHandlerHook,
                            false);
    HookManager::Initialize(HookID::kBattleGetMoveHandler,
                            0x00762B18,
                            (uptr)GetBattleMoveHandlerHook,
                            false);
    HookManager::Initialize(HookID::kLoadMoveData,
                            ADDRESS_LOAD_MOVE_DATA,
                            (uptr)LoadMoveData);
  }

  static u32 LoadMoveData(uptr self, u16 move_id) {
    u32 id = move_id;
    if (move_id == MOVE_ABSOLUTE_ZERO || move_id == MOVE_SOLAR_FLARE) id = 1;
    u32 result = HookManager::Call<u32>(HookID::kLoadMoveData, self, id);
    if (move_id == MOVE_ABSOLUTE_ZERO) {
      auto& move = *(global_data::Move*)(READ32(self + 8));
      move.power = 0;
      move.accuracy = 100;
      move.base_pp = 5;
      move.type = MOVE_TYPE_ICE;
      move.effect_id = STATUS_CONDITION_FREEZE;
      move.effect_rate = 100;
      move.effect_turn_type = 1;
      move.flinch_rate = 100;
      move.category = 4;
      move.damage_category = 1;
      WRITE16(self + 4, move_id);
    }
    if (move_id == MOVE_SOLAR_FLARE) {
      auto& move = *(global_data::Move*)(READ32(self + 8));
      move.power = 0;
      move.accuracy = 100;
      move.base_pp = 5;
      move.type = MOVE_TYPE_FIRE;
      move.effect_id = STATUS_CONDITION_BURN;
      move.effect_rate = 100;
      move.effect_turn_type = 1;
      move.flinch_rate = 100;
      move.category = 4;
      move.damage_category = 1;
      WRITE16(self + 4, move_id);
    }
    return result;
  }

  static bool PatchMoveName(u16 move, String* output) {
    switch (move) {
      case MOVE_ABSOLUTE_ZERO:
        output->Set(u"Absolute Zero");
        return true;
      case MOVE_SOLAR_FLARE:
        output->Set(u"Solar Flare");
        return true;
      default:
        return false;
    }
  }

  static bool PatchMoveDescription(u16 move, String* output) {
    switch (move) {
      case MOVE_ABSOLUTE_ZERO:
        output->Set(
            u"Instantly freezes the target solid.");
        return true;
      case MOVE_SOLAR_FLARE:
        output->Set(
            u"Summons blinding sunlight\nand instantly leaves the target with a severe burn.");
        return true;
      default:
        return false;
    }
  }

  static bool PatchAbilityName(u8 ability, String* output) {
    switch (ability) {
      case ABILITY_RANDOM:
        output->Set(u"Random");
        return true;
      default:
        return false;
    }
  }

  static bool PatchAbilityDescription(u8 ability, String* output) {
    switch (ability) {
      case ABILITY_RANDOM:
        output->Set(u"???");
        return true;
      default:
        return false;
    }
  }

  STATIC_INLINE void PatchBattleLoad() {
    HookManager::ForceEnable(HookID::kBattleGetAbilityHandler);
    HookManager::ForceEnable(HookID::kBattleGetMoveHandler);
  }

  static u32 GetValue(u32 var) {
    return ((u32(*)(u32))0x00764668)(var);
  }

  static void UpdateWeather(uptr manager, u8 uid, u8 weather, u16 item,
                            bool infinite) {
    ((void(*)(uptr, u8, u8, u16, bool))0x00741CC0)
        (manager, uid, weather, item, infinite);
  }

  static void handler_random(uptr a, uptr manager, u32 uid, uptr c) {
    static u8 weather = 0;
    weather++;
    if (weather == 6) weather = 0;
    UpdateWeather(manager, uid, 1 + weather, 0, true);

    // imposter
    // ((void(*)(uptr, uptr, u32, uptr))0x7B9568)(a, manager, uid, c);
  }

  static void handler_sun(uptr a, uptr manager, u32 uid, uptr c) {
    UpdateWeather(manager, uid, WEATHER_BATTLE_HARSH_SUNLIGHT, 0, true);
  }

  static uptr GetBattleAbilityHandlerHook(battle::Pokemon* pkm) {
    static struct {
      u32 event;
      uptr handler;
    } table[] = {
        {94, (uptr)handler_random}
    };
    if (pkm->ability == ABILITY_RANDOM) {
      return ((uptr(*)(u32, u32, u32, u32, u32, u32, u32))0x00743EFC)(
          4, pkm->ability, 0, 1000, pkm->uid, (uptr)table, SIZE(table));
    }
    return HookManager::Call<uptr>(HookID::kBattleGetAbilityHandler, pkm);
  }

  static uptr GetBattleMoveHandlerHook(battle::Pokemon* pkm, u16 move, u32 x) {
    ui::LogApplication::Print(u"Move Handler %u", move);
    static struct {
      u32 event;
      uptr handler;
    } table[] = {
        {39, (uptr)handler_sun}
    };
    if (move == MOVE_SOLAR_FLARE) {
      return ((uptr(*)(u32, u32, u32, u32, u32, u32, u32))0x00743EFC)(
          0, move, 0, x, pkm->uid, (uptr)table, SIZE(table));
    }
    return HookManager::Call<uptr>(HookID::kBattleGetMoveHandler, pkm);
  }

  static void MessageGetStringHook(Message* self, u32 str_id, String* output) {
    HookManager::Call<void>(HookID::kMessageGetString, self, str_id, output);
    switch (self->file_id) {
      case 14:
      case 15:
        PatchMoveName(str_id, output);
        break;
      case 16:
        PatchMoveDescription(str_id, output);
        break;
      case 36:
        PatchAbilityDescription(str_id, output);
        break;
      case 37:
        PatchAbilityName(str_id, output);
        break;
      default:
        break;
    }
  }

  static void SetAbilityNameHook(uptr self, u32 archive, u32 ability) {
    HookManager::Call<void>(HookID::kSetAbilityName, self, archive, ability);
    String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
    PatchAbilityName(ability, output);
  }

  static void SetMoveNameHook(uptr self, u32 archive, u32 move) {
    HookManager::Call<void>(HookID::kSetMoveName, self, archive, move);
    String* output = (String*)READ32(READ32(self + 8) + 12 * archive);
    PatchMoveName(move, output);
  }

  static void GetAbilityNameHook(String* output, u8 ability) {
    if (PatchAbilityName(ability, output)
        || ability >= ABILITY_COUNT)
      return;

    GetAbilityName().GetString(ability, output);
  }

  static void GetMoveNameHook(u16 move, String* output) {
    if (PatchMoveName(move, output)
        || move >= MOVE_COUNT)
      return;

    GetMoveName().GetString(move, output);
  }

  static void GetAbilityDescriptionHook(String* output, u8 ability) {
    if (PatchAbilityDescription(ability, output)
        || ability >= ABILITY_COUNT)
      return;

    GetAbilityDescription().GetString(ability, output);
  }

  STATIC_INLINE Message& GetAbilityName() {
    return *(Message*)READ32(0x00617A0C);
  }

  STATIC_INLINE Message& GetAbilityDescription() {
    return *(Message*)READ32(0x00617A10);
  }

  STATIC_INLINE Message& GetMoveName() {
    return *(Message*)READ32(0x00617A6C);
  }
};
}