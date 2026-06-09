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
#include "game/manager.h"

namespace global_data {
struct Item {
  INLINE Item(const u16 id) {
    ((void(*)(Item*, u16, void*))ADDRESS_GLOBAL_DATA_ITEM_INITIALIZE)
        (this, id, game::Manager::GetInstance().GetSystemHeap());
  }

  INLINE void GetName(String* str) {
    ((void(*)(Item*, String*, void*))ADDRESS_GLOBAL_DATA_ITEM_GET_NAME)
        (this, str, game::Manager::GetInstance().GetSystemHeap());
  }

  u16 price;
  u8 hold_effect;
  u8 power;

  u8 pluck_effect;
  u8 fling_effect;
  u8 fling_power;
  u8 natural_gift_power;

  u16 natural_gift_type : 5;
  u16 is_key_item : 1;
  u16 registered_button : 1;
  u16 field_pocket : 4;
  u16 battle_pocket : 5;
  u8 field_function;
  u8 battle_function;

  bool use_on_pokemon;
  u8 item_type;
  u8 battle_hold_consume : 4;
  u8 use_no_consume : 4;

  u8 sort_number;

  u8 cure_sleep : 1;
  u8 cure_poison : 1;
  u8 cure_burn : 1;
  u8 cure_freeze : 1;
  u8 cure_paralysis : 1;
  u8 cure_confusion : 1;
  u8 cure_infatuation : 1;
  u8 guard_stat_stages : 1;

  u8 revive : 1;
  u8 revive_all : 1;
  u8 level_up : 1;
  u8 evolve : 1;
  u8 raise_attack : 4;

  u8 raise_defense : 4;
  u8 raise_sp_atk : 4;

  u8 raise_sp_def : 4;
  u8 raise_speed : 4;

  u8 raise_accuracy : 4;
  u8 raise_critical_hit : 2;
  u8 use_pp_up : 1;
  u8 use_pp_max : 1;

  u8 restore_pp : 1;
  u8 restore_all_pp : 1;
  u8 restore_hp : 1;
  u8 raise_hp_ev : 1;
  u8 raise_attack_ev : 1;
  u8 raise_defense_ev : 1;
  u8 raise_speed_ev : 1;
  u8 raise_sp_atk_ev : 1;

  u8 raise_sp_def_ev : 1;
  u8 ev_limit_control : 1;
  u8 friendship_mod_1 : 1;
  u8 friendship_mod_2 : 1;
  u8 friendship_mod_3 : 1;

  u8 _0 : 3;

  s8 hp_ev_value;
  s8 attack_ev_value;
  s8 defense_ev_value;
  s8 speed_ev_value;
  s8 sp_atk_ev_value;
  s8 sp_def_ev_value;
  u8 hp_restore_value;
  u8 pp_restore_value;
  s8 friendship_value_1;
  s8 friendship_value_2;
  s8 friendship_value_3;

  u8 _1[2];

  u32 id;
};
} // namespace global_data