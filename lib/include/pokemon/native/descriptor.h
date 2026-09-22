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
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"
#include "pokemon/native/core_data.h"

namespace pokemon {
struct Descriptor {
  static constexpr u64 kRandomId = 0xFFFFFFFFFFFFFFFFull;
  static constexpr u64 kRandomShiny = 0x00000003FFFFFFFFull;
  static constexpr u64 kNotShiny = 0x00000001FFFFFFFFull;
  static constexpr u64 kShiny = 0x00000002FFFFFFFFull;
  static constexpr u16 kRandomSex = 0xFF;
  static constexpr u16 kRandomNature = 0xFFFF;
  static constexpr u8 kAbility1Or2 = 0xFF;
  static constexpr u16 kRandomIv = 0xFFFF;
  static constexpr u32 kDefaultFriendship = 0xFFFF;

  Descriptor(SpeciesId species, u16 level,
             FormId form = FormId::kNormal) : _0(0) {
    personal_random = kRandomId;
    shiny_random = kRandomShiny;
    id = kRandomId;
    this->species = species;
    this->form = form;
    this->level = level;
    sex = kRandomSex;
    nature = kRandomNature;
    ability_index = kAbility1Or2;
    shiny_try_count = 1;
    for (u32 i = 0; i < 6; i++) ivs[i] = kRandomIv;
    friendship = kDefaultFriendship;
    perfect_iv_count = 0;
  }

  void Create(CoreData* core) const {
    ((void (*)(CoreData*, const Descriptor*))
      address::kCreateCoreDataFromDescriptor)(
        core, this);
  }

  u64 personal_random;
  u64 shiny_random;
  u64 id;
  SpeciesId species;
  FormId form;
  u8 _0;
  u16 level;
  u16 sex;
  u16 nature;
  u8 ability_index;
  u8 shiny_try_count;
  u16 ivs[6];
  u32 friendship;
  u8 perfect_iv_count;
};
} // namespace pokemon