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
#include "game/constant/ability.h"
#include "game/constant/form.h"
#include "game/constant/item.h"
#include "game/constant/pokemon_type.h"
#include "game/constant/species.h"

namespace global_data {
struct Pokemon {
  STATIC_INLINE Pokemon& GetInstance(const Species species,
                                     const Form form = static_cast<Form>(0)) {
    Pokemon* table = (Pokemon*)READ32(ADDRESS_GLOBAL_DATA_POKEMON_TABLE);

    // cf. 0x0014EAA0
    const u32 species_index = static_cast<u16>(species);
    const u32 form_index = static_cast<u8>(form);
    u32 index = species_index;
    if (form_index != 0 && table[species_index].form_index != 0
        && form_index < table[species_index].form_count) {
      index = table[species_index].form_index + form_index - 1;
    }

    return table[index];
  }

  u8 base_hp;
  u8 base_attack;
  u8 base_defense;
  u8 base_speed;
  u8 base_special_attack;
  u8 base_special_defense;
  PokemonType type[2];
  u8 capture_rate;
  u8 _0;
  u16 give_effort_values;
  ItemId give_item[3];
  u8 gender; // gender *ratio* (0-254 threshold), not a Gender id
  u8 egg_hatch_steps;
  u8 base_friendship;
  u8 _1;
  u8 egg_group[2];
  Ability ability[3];
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