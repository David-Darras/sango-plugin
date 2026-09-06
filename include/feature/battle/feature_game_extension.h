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
#include "feature/weather_manager.h"
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
constexpr Ability kAbilityToxicDrizzle = static_cast<Ability>(255);
constexpr Ability kAbilityRadioactiveDrizzle = static_cast<Ability>(254);
constexpr Ability kAbilityRealityWarp = static_cast<Ability>(253);
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
    HookManager::Initialize(HookID::kBattleAddTerrain,
                            0x00718B10,
                            (uptr)BattleAddTerrainHook,
                            false);
    HookManager::Initialize(HookID::kLoadMoveData,
                            ADDRESS_LOAD_MOVE_DATA,
                            (uptr)LoadMoveData);
  }

  static u32 BattleAddTerrainHook(u32 a, u32 b) {
    u32 res = HookManager::Call<u32>(HookID::kBattleAddTerrain, a, b);
    ui::LogApplication::Print(u"res=%u", res);
    return res;
  }

  // `id` is polymorphic here (a move id only when `is_move` is set,
  // something else otherwise), so it stays a raw u32.
  static void BattleLoadAnimationHook(uptr self, u32 id, bool is_move) {
    // ui::LogApplication::Print(u"Anim(%u, %u)", id, is_move);
    if (is_move) {
      if (id == static_cast<u32>(kMoveAbsoluteZero)) {
        id = 19;
        is_move = false;
      }
      if (id == static_cast<u32>(kMoveSolarFlare)) {
        id = 18;
        is_move = false;
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
      move.base_pp = 50;
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
      move.base_pp = 50;
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
            u"Summons a hailstorm\nand instantly freezes the target solid.");
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
      case kAbilityToxicDrizzle:
        output->Set(u"Toxic Drizzle");
        return true;
      case kAbilityRadioactiveDrizzle:
        output->Set(u"Radioactive Drizzle");
        return true;
      case kAbilityRealityWarp:
        output->Set(u"Reality Warp");
        return true;
      default:
        return false;
    }
  }

  static bool PatchAbilityDescription(Ability ability, String* output) {
    switch (ability) {
      case kAbilityToxicDrizzle:
        output->Set(u"Summons acid rain that\npoisons all Pokémon on entry.");
        return true;
      case kAbilityRadioactiveDrizzle:
        output->Set(
            u"Summons a radioactive rain\nthat triggers Imposter on entry.");
        return true;
      case kAbilityRealityWarp:
        output->Set(
            u"???");
        return true;
      default:
        return false;
    }
  }

  STATIC_INLINE void PatchBattleLoad() {
    HookManager::ForceEnable(HookID::kBattleGetAbilityHandler);
    HookManager::ForceEnable(HookID::kBattleGetMoveHandler);
    HookManager::ForceEnable(HookID::kBattleLoadAnimation);
    HookManager::ForceEnable(HookID::kBattleAddTerrain);
  }

  static u32 GetValue(u32 var) {
    return ((u32(*)(u32))ADDRESS_BATTLE_GET_VARIABLE)(var);
  }

  static void UpdateWeather(uptr manager, u8 uid, BattleWeather weather,
                            ItemID item, bool infinite) {
    ((void(*)(uptr, u8, BattleWeather, ItemID, bool))
      ADDRESS_BATTLE_UPDATE_WEATHER)(manager, uid, weather, item, infinite);
  }

  static void handler_toxic(uptr a, uptr manager, u32 uid, uptr c) {
    feature::WeatherManager::GetInstance().mode = feature::WeatherMode::kToxic;
    UpdateWeather(manager, uid, BattleWeather::kRain, ItemID::kNone, true);

    {
      uptr p = ((uptr(*)(uptr, u32, u32))0x00762CC0)(manager, 11, 12);
      WRITE8(p + 4, 5);
      WRITE32(p + 8, 1);
      WRITE32(p + 12, 0);
      WRITE8(p + 15, 12);
      ((void(*)(uptr, uptr))0x0076088C)(manager, p);
    }
  }

  static void handler_radioactive(uptr a, uptr manager, u32 uid, uptr c) {
    WeatherManager::GetInstance().mode = WeatherMode::kRadioactive;
    UpdateWeather(manager, uid, BattleWeather::kRain, ItemID::kNone, true);

    struct HP {
      u32 _0;
      u8 count;
      u8 _1, _2;
      u8 uid[10];
      s32 amount[10];
    };

    {
      auto* pkm1 = ((battle::Pokemon*(*)(uptr, u32))0x0074BC4C)(manager, uid);
      auto* pkm2 = ((battle::Pokemon*(*)(uptr, u32))0x0074BC4C)(manager, 12);

      HP* p = ((HP*(*)(uptr, u32, u32))0x00762CC0)(manager, 7, uid);
      p->count = 2;
      p->uid[0] = pkm1->uid;
      p->amount[0] = -(pkm1->hp - 1);
      p->uid[1] = pkm2->uid;
      p->amount[1] = -(pkm2->hp - 1);
      ((void(*)(uptr, HP*))0x0076088C)(manager, p);
    }

    struct FORM {
      u32 _0;
      u8 id;
      Form form;
    };

    {
      FORM* p = ((FORM*(*)(uptr, u32, u32))0x00762CC0)(manager, 58, uid);
      p->id = uid;
      p->form = Form::kRotomFan;
      ((void(*)(uptr, FORM*))0x0076088C)(manager, p);
    }

    // imposter
    // ((void(*)(uptr, uptr, u32, uptr))0x7B9568)(a, manager, uid, c);
  }

  static void attack(uptr manager, u32 uid, MoveID move_id) {
    auto* attacker = ((battle::Pokemon*(*)(uptr, u32))0x0074BC4C)(manager, uid);

    struct {
      u32 kind : 4;
      u32 target : 4;
      u32 move_id : 16;
      u32 _0 : 3;
      u32 _1 : 1;
      u32 mega_evolution : 1;
      u32 _2 : 3;
    } action;

    action.kind = 1;
    action.target = 0;
    action.move_id = static_cast<u16>(move_id);

    ((void(*)(uptr, uptr, uptr, u32, s32))0x0070EC48)(
        manager, (uptr)attacker, (uptr)&action, 0, 0);
  }

  static void handler_reality_warp(uptr a, uptr manager, u32 uid, uptr c) {
    u32 id = ((u32(*)(u32))0x00764668)(2);
    if (id != uid) return;
    attack(manager, uid, MoveID::kTrickRoom);
    attack(manager, uid, MoveID::kWonderRoom);
    attack(manager, uid, MoveID::kMagicRoom);
    attack(manager, uid, MoveID::kGravity);
    attack(manager, uid, MoveID::kGrassyTerrain);
    attack(manager, uid, MoveID::kStealthRock);
  }

  static void handler_sun(uptr a, uptr manager, u32 uid, uptr c) {
    UpdateWeather(manager, uid, BattleWeather::kHarshSunlight, ItemID::kNone,
                  true);
  }

  static void handler_hail(uptr a, uptr manager, u32 uid, uptr c) {
    UpdateWeather(manager, uid, BattleWeather::kHail, ItemID::kNone,
                  true);
  }

  static uptr GetBattleAbilityHandlerHook(battle::Pokemon* pkm) {
    static struct {
      u32 event;
      uptr handler;
    } toxic_table[] = {
        {94, (uptr)handler_toxic}
    };
    static struct {
      u32 event;
      uptr handler;
    } radioactive_table[] = {
        {94, (uptr)handler_radioactive}
    };
    static struct {
      u32 event;
      uptr handler;
    } reality_warp_table[] = {
        {94, (uptr)handler_reality_warp}
    };
    if (pkm->ability == kAbilityToxicDrizzle) {
      return ((uptr(*)(u32, Ability, u32, u32, u32, u32, u32))
        ADDRESS_BATTLE_GET_EVENT_HANDLER)(
          4, pkm->ability, 0, 1000, pkm->uid, (uptr)toxic_table,
          SIZE(toxic_table));
    }
    if (pkm->ability == kAbilityRadioactiveDrizzle) {
      return ((uptr(*)(u32, Ability, u32, u32, u32, u32, u32))
        ADDRESS_BATTLE_GET_EVENT_HANDLER)(
          4, pkm->ability, 0, 1000, pkm->uid, (uptr)radioactive_table,
          SIZE(radioactive_table));
    }
    if (pkm->ability == kAbilityRealityWarp) {
      return ((uptr(*)(u32, Ability, u32, u32, u32, u32, u32))
        ADDRESS_BATTLE_GET_EVENT_HANDLER)(
          4, pkm->ability, 0, 1000, pkm->uid, (uptr)reality_warp_table,
          SIZE(reality_warp_table));
    }
    return HookManager::Call<uptr>(HookID::kBattleGetAbilityHandler, pkm);
  }

  static uptr GetBattleMoveHandlerHook(battle::Pokemon* pkm, MoveID move,
                                       u32 x) {
    if (move == kMoveAbsoluteZero) {
      static struct {
        u32 event;
        uptr handler;
      } table[] = {
          {216, (uptr)handler_hail}
      };
      return ((uptr(*)(u32, MoveID, u32, u32, u32, u32, u32))
        ADDRESS_BATTLE_GET_EVENT_HANDLER)(
          0, move, 0, x, pkm->uid, (uptr)table,
          SIZE(table));
    }
    if (move == kMoveSolarFlare) {
      static struct {
        u32 event;
        uptr handler;
      } table[] = {
          {216, (uptr)handler_sun}
      };
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