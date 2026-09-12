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

#include "pokemon_core_data.h"
#include "game/savedata/savedata.h"

namespace savedata {
struct DayCare {
  SINGLETON(DayCare)
  STATIC_INLINE DayCare& GetInstance() {
    return SaveData::GetInstance().GetDayCare();
  }

  struct Pokemon {
    u32 _0;
    u32 exp;
    PokemonCoreData data;
  };

  struct Location {
    Pokemon pokemon[2];
    bool is_egg_available;
    u64 random;
  };

  void* vtable;
  Location location[2];
};
} // namespace savedata