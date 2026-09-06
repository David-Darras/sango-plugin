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
#include "game/constant/form.h"
#include "game/constant/move.h"

namespace global_data {
struct GiftPokemon {
  STATIC_INLINE GiftPokemon& GetInstance(u32 idx) {
    return *(GiftPokemon*)(
      ADDRESS_GIFT_POKEMON_TABLE + sizeof(GiftPokemon) * idx);
  }

  enum class Shiny : u8 { kRandom, kForceShiny, kForceNotShiny };

  enum class GenderRoll : u8 { kRandom, kMale, kFemale };

  static constexpr u16 kNotAnEgg = 0xFFFF;
  static constexpr s8 kRandomAbility = -1;
  static constexpr s8 kRandomNature = -1;
  static constexpr s32 kRandomItem = -1;
  static constexpr s8 kRandomIv = -1;

  u32 species;
  Form form;
  u8 level;
  Shiny shiny;
  s8 ability_slot;
  s8 nature;
  s32 item;
  GenderRoll gender;
  u16 egg_place;
  MoveId move;
  s8 iv[6];
  u8 contest[6];
};
}