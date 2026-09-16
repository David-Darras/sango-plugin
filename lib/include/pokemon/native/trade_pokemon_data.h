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
#include "pokemon/constant/item.h"
#include "pokemon/constant/nature.h"

namespace pokemon {
struct TradePokemonData {
  STATIC_INLINE TradePokemonData& GetInstance(u32 idx) {
    return *(TradePokemonData*)(
      address::kTradePokemonTable + sizeof(TradePokemonData) * idx);
  }

  SpeciesId species;
  u8 nickname_id;
  FormId form;

  u8 level;
  u8 ev_hp;
  u8 ev_attack;
  u8 ev_defense;

  u8 ev_speed;
  u8 ev_special_attack;
  u8 ev_special_defense;
  u8 ability_idx;

  Nature nature;
  Gender gender;
  u16 _0;

  ItemId item;
  u8 parent_nickname_id;
  Gender parent_gender;

  u8 _3[12];

  SpeciesId wanted_species;
  Gender wanted_gender;
  u8 _4;
};
} // namespace pokemon
