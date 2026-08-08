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

namespace global_data {
struct Pokemon {
  STATIC_INLINE Pokemon& GetInstance(const u16 species, const u8 form) {
    Pokemon* table = (Pokemon*)READ(u32, ADDRESS_GLOBAL_DATA_POKEMON_TABLE);

    // cf. 0x0014EAA0
    u32 index = species;
    if (form != 0 && table[species].form_index != 0 && form < table[species].
        form_count) {
      index = table[species].form_index + form - 1;
    }

    return table[index];
  }

  u8 base_hp;
  u8 base_attack;
  u8 base_defense;
  u8 base_speed;
  u8 base_special_attack;
  u8 base_special_defense;
  u8 type[2];
  u8 capture_rate;
  u8 _0;
  u16 give_effort_values;
  u16 give_item[3];
  u8 gender;
  u8 egg_hatch_steps;
  u8 base_friendship;
  u8 _1;
  u8 egg_group[2];
  u8 ability[3];
  u8 escape_rate;
  u16 form_index;
  u16 form_index_2;
  u8 form_count;
  u8 _2;
  u16 give_experience;
  u16 height;
  u16 weight;
  u32 technical_moves[5];
  u16 fake_height;
  u16 _4;
  u32 _5[4];
};
} // namespace global_data