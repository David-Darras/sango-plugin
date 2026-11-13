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
#include "pokemon/constant/pokedex_color.h"
#include "pokemon/constant/egg_group.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/leveling_rate.h"
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
        && form_index < table[species_index].total_form_count) {
      index = table[species_index].form_index + form_index - 1;
    }

    return table[index];
  }

  /// The base stats.
  u8 base_hp;
  u8 base_attack;
  u8 base_defense;
  u8 base_speed;
  u8 base_special_attack;
  u8 base_special_defense;
  /// The primary type.
  TypeId type_1;
  /// The secondary type.
  TypeId type_2;
  /// The catch rate.
  u8 catch_rate;
  u8 unused_0;
  /// The effort values and entry flag:
  /// bits 0..11: EV yield (2 bits each for HP, Atk, Def, Spe, SpA, SpD);
  /// bit 12: burrowed entry flag (1 = emerges directly in the ground like Diglett/Dugtrio, 0 = standard drop from Poké Ball);
  /// bits 13..15: unused.
  u16 ev_yield;
  /// Common held item present on 50% of wild encounters.
  ItemId held_item_50;
  /// Rare held item present on 5% of wild encounters.
  ItemId held_item_5;
  ItemId unused_1;
  /// The gender ratio: 0 male only, 254 female only, 255 no gender.
  u8 gender_ratio;
  /// The number of egg cycles to hatch an Egg. 1 cycle = 256 steps.
  u8 hatch_time;
  u8 base_friendship;
  /// Growth curve identifier / Leveling rate
  LevelingRate leveling_rate;
  /// The primary Egg Group.
  EggGroup egg_group_1;
  /// The secondary Egg Group.
  EggGroup egg_group_2;
  /// The two abilities and the hidden ability.
  AbilityId ability[3];
  u8 unused_2;
  u16 form_index; ///< The index of the first form in the table, or 0.
  /// The index of the first model of the other forms in the table, or 0. The
  /// game reads it next to form_index: both must move together.
  u16 form_model_index;
  /// The number of total forms.
  u8 total_form_count;
  PokedexColor color;
  /// The base experience yield.
  u16 base_experience;
  /// The height in centimeters.
  u16 height;
  /// The weight in hectograms.
  u16 weight;
  u32 technical_moves[5];
  ///< The TMs and HMs that the species can learn (one bit each).
  u16 unused_4; ///< fake height
  u16 _4;
#ifndef GAME_XY
  u32 _5[4];
#endif
};

static_assert(sizeof(SpeciesData) == GAME_CONSTANT(0x40, 0x50),
              "SpeciesData must match the game's personal data layout");
} // namespace pokemon