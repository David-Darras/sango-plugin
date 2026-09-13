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
#include <types.h>

namespace battle {
// Which of the layouts below `raw` should be read through, based on its low 3 bits.
enum class StatusDataKind : u8 {
  kTurnBased, ///< Wears off after a fixed number of turns
  kPokemonBound, ///< Tied to a specific Pokémon id, not a turn count
  kPermanent, ///< Lasts until explicitly cured, optionally counting up to a max
  kTurnAndPokemonBound, ///< Both a turn count and a bound Pokémon id
};

union StatusData {
  u32 raw;

  struct {
    StatusDataKind kind : 3;
    u32 bound_pokemon_id : 6;
    u32 param : 16;
    u32 flag : 1;
    u32 _padding : 6;
  } pokemon_bound;

  struct {
    StatusDataKind kind : 3;
    u32 turns_remaining : 6;
    u32 param : 16;
    u32 flag : 1;
    u32 _padding : 6;
  } turn_based;

  struct {
    StatusDataKind kind : 3;
    u32 turns_remaining : 6;
    u32 bound_pokemon_id : 6;
    u32 param : 16;
    u32 flag : 1;
  } turn_and_pokemon_bound;

  struct {
    StatusDataKind kind : 3;
    u32 max_turn_count : 6;
    u32 param : 16;
    u32 flag : 1;
    u32 _padding : 6;
  } permanent;
};
}
