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
#include "game/constant/item.h"
#include "game/global_data/item.h"
#include "ui/log_application.h"
#include <CTRPluginFramework/System/Hook.hpp>

#include "kaizo.h"

namespace feature {
class Item {
  MAKE_SINGLETON(Item)
  bool remove_limit = false;

  STATIC_INLINE void Initialize() {
    auto& feat = Item::GetInstance();

    // No level limit with rare candy
    // WRITE(vu32, 0x0046DED8, 0xE35000FF);

    if (feat.remove_limit) {
      // No limit to 100 for EVs with ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, etc.
      WRITE(vu32, 0x0047888C, 0xE35000FF);
      WRITE(vu32, 0x004788F0, 0xE35000FF);
      WRITE(vu32, 0x004788FC, 0x03A000FF);
      WRITE(vu32, 0x0046E104, 0xE35B00FF);
      WRITE(vu32, 0x0046E1A4, 0xE35B00FF);
      WRITE(vu32, 0x0046E244, 0xE35800FF);
      WRITE(vu32, 0x0046E2E0, 0xE35900FF);
      WRITE(vu32, 0x0046E37C, 0xE35A00FF);
      WRITE(vu32, 0x0046E41C, 0xE35A00FF);
    }

    HookManager::Initialize(HookID::kGlobalDataItemGetParam,
                            ADDRESS_GLOBAL_DATA_ITEM_GET_PARAM,
                            (uptr)GetParamHook);
  }

  /* The first instructions use the pc register so we have to rewrite all the code */
  static u32 GetParamHook(global_data::Item* item, u32 param_id) {
    if (!item) return 0;

#ifdef KAIZO
    kaizo::PatchItemData(item);
#endif

    return GetParam(item, param_id);
  }

  static s32 GetParam(const global_data::Item* item, s32 param_id) {
    if (!item) return 0;

    switch (param_id) {
      case 0:
        return item->id;
      case 1:
        return item->price;
      case 2:
        return item->hold_effect;
      case 3:
        return item->power;
      case 4:
        return item->pluck_effect;
      case 5:
        return item->fling_effect;
      case 6:
        return item->fling_power;
      case 7:
        return item->natural_gift_power;
      case 8:
        return item->natural_gift_type;
      case 9:
        return item->is_key_item;
      case 10:
        return item->registered_button;
      case 11:
        return item->field_pocket;
      case 12:
        return item->battle_pocket;
      case 13:
        return item->field_function;
      case 14:
        return item->battle_function;
      case 15:
        return item->use_on_pokemon;
      case 16:
        return item->item_type;
      case 17:
        return item->battle_hold_consume;
      case 18:
        return item->use_no_consume;
      case 19:
        return item->sort_number;
      default:
        break;
    }

    if (item->use_on_pokemon) {
      switch (param_id) {
        case 20:
          return item->cure_sleep;
        case 21:
          return item->cure_poison;
        case 22:
          return item->cure_burn;
        case 23:
          return item->cure_freeze;
        case 24:
          return item->cure_paralysis;
        case 25:
          return item->cure_confusion;
        case 26:
          return item->cure_infatuation;
        case 27:
          return item->guard_stat_stages;
        case 28:
          return item->revive;
        case 29:
          return item->revive_all;
        case 30:
          return item->level_up;
        case 31:
          return item->evolve;
        case 32:
          return item->raise_attack;
        case 33:
          return item->raise_defense;
        case 34:
          return item->raise_sp_atk;
        case 35:
          return item->raise_sp_def;
        case 36:
          return item->raise_speed;
        case 37:
          return item->raise_accuracy;
        case 38:
          return item->raise_critical_hit;
        case 39:
          return item->use_pp_up;
        case 40:
          return item->use_pp_max;
        case 41:
          return item->restore_pp;
        case 42:
          return item->restore_all_pp;
        case 43:
          return item->restore_hp;
        case 44:
          return item->raise_hp_ev;
        case 45:
          return item->raise_attack_ev;
        case 46:
          return item->raise_defense_ev;
        case 47:
          return item->raise_speed_ev;
        case 48:
          return item->raise_sp_atk_ev;
        case 49:
          return item->raise_sp_def_ev;
        case 50:
          return item->ev_limit_control;
        case 51:
          return item->friendship_mod_1;
        case 52:
          return item->friendship_mod_2;
        case 53:
          return item->friendship_mod_3;
        case 54:
          return item->hp_ev_value;
        case 55:
          return item->attack_ev_value;
        case 56:
          return item->defense_ev_value;
        case 57:
          return item->speed_ev_value;
        case 58:
          return item->sp_atk_ev_value;
        case 59:
          return item->sp_def_ev_value;
        case 60:
          return item->hp_restore_value;
        case 61:
          return item->pp_restore_value;
        case 62:
          return item->friendship_value_1;
        case 63:
          return item->friendship_value_2;
        case 64:
          return item->friendship_value_3;
        default:
          break;
      }
    }

    return 0;
  }
};
}