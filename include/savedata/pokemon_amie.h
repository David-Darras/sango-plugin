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
#include "savedata.h"

namespace savedata {
/**
* @brief Manages the Pokémon-Amie (Kawaigari) system data.
* * This structure handles the inventory of Poké Puffs and global
* interaction timestamps for the Amie system in Pokémon ORAS.
*/
struct PokemonAmie {
  STATIC_INLINE PokemonAmie& GetInstance() {
    return SaveData::GetInstance().GetPokemonAmie();
  }

  static constexpr u32 kMaxPokePuffId = 24;
  static constexpr u32 kMaxPokePuffs = 100;

  void* vtable;
  u8 poke_puffs[kMaxPokePuffs];
  s32 last_opened_timestamp;
};
} // namespace savedata