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
 * @file species_data.h
 * @brief The data of one species: base stats, types, abilities...
 */

#pragma once

#include "common.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/type.h"
#include "pokemon/constant/species.h"
#include "pokemon/native/database.h"

namespace pokemon {
/// The data of one species or of one form: base stats, types, abilities, gender ratio...
struct SpeciesData {
  /// Returns the table of all the species and forms.
  STATIC_INLINE SpeciesData* GetTable() {
    return Database::GetInstance().species;
  }

  /// Returns the data of a species and form.
  STATIC_INLINE SpeciesData& GetInstance(const SpeciesId species,
                                         const FormId form = FormId::kNormal) {
    SpeciesData* table = GetTable();

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
  TypeId type[2];
  u8 capture_rate; ///< The catch rate.
  u8 _0;
  u16 give_effort_values; ///< The EVs that the Pokémon gives when it faints.
  ItemId give_item[3]; ///< The items that the wild Pokémon can hold.
  u8 gender; ///< The gender ratio (0 male only, 254 female only, 255 no gender).
  u8 egg_hatch_steps; ///< The steps to hatch an Egg, in cycles.
  u8 base_friendship;
  u8 _1;
  u8 egg_group[2]; ///< The Egg Groups.
  AbilityId ability[3]; ///< The two abilities and the hidden ability.
  u8 escape_rate;
  u16 form_index; ///< The index of the first form in the table, or 0.
  u16 form_index_2;
  u8 form_count; ///< The number of forms.
  u8 _2;
  u16 give_experience; ///< The base experience yield.
  u16 height; ///< The height in decimeters.
  u16 weight; ///< The weight in hectograms.
  u32 technical_moves[5]; ///< The TMs and HMs that the species can learn (one bit each).
  u16 fake_height;
  u16 _4;
#ifndef GAME_XY
  u32 _5[4];
#endif
};
static_assert(sizeof(SpeciesData) == GAME_CONSTANT(0x40, 0x50),
              "SpeciesData must match the game's personal data layout");
} // namespace pokemon
