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

#define ADDRESS_GIFT_POKEMON_TABLE (0x007EC06C)

namespace global_data {
struct GiftPokemon {
  STATIC_INLINE GiftPokemon& GetInstance(u32 idx) {
    if (idx >= 37) idx = 0;
    GiftPokemon* table = (GiftPokemon*)ADDRESS_GIFT_POKEMON_TABLE;
    return table[idx];
  }

  u32 species;
  u8 form;
  u8 level;
  u8 shiny;
  u8 ability;
  u8 nature;
  u32 item;
  u8 gender;
  u16 is_egg;
  u16 move;

  struct {
    u8 hp, attack, defense, special_attack, special_defense, speed;
  } iv;

  struct {
    u8 cool, beautiful, cute, smart, tough, sheen;
  } contest;
};
}