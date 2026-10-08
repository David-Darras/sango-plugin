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
 * @file status_data.h
 * @brief The duration data of a status condition.
 */

#pragma once
#include <types.h>

namespace battle {
/// The layout of a StatusData. The 3 low bits of `raw` select it.
enum class StatusDataKind : u8 {
  kTurnBased, ///< Stops after a number of turns.
  kPokemonBound, ///< Linked to a Pokémon, not to a number of turns.
  kPermanent, ///< Stays until a cure. It can count up to a maximum.
  kTurnAndPokemonBound, ///< A number of turns and a linked Pokémon.
};

/// The duration of a status condition: turns, a linked Pokémon, or both.
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
