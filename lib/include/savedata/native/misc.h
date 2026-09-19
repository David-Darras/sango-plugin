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
#include "savedata/native/savedata.h"

namespace savedata {
struct Misc {
  SINGLETON(Misc)
  STATIC_INLINE Misc& GetInstance() {
    return SaveData::GetInstance().GetMisc();
  }

  INLINE u32 GetBadgesCount() {
    u32 count = 0;
    for (u32 i = 0; i < 8; i++) {
      if (badges & (1 << i)) count++;
    }
    return count;
  }

  static constexpr u32 kNicknameLength = 13;

  void* vtable;
  u32 padding0;
  u64 random_value;

  u32 money;
  u32 badges;

  c16 rival_nickname[kNicknameLength];
  c16 padding1;

  u32 flags;
#ifdef GAME_XY
  u32 tutorial_pokemon_amie;
  u32 tutorial_super_training;
  u32 flags2;
#endif
  u32 battle_points;

  /**
* @brief Bitmask tracking encountered Mega Evolutions.
* Each bit represents a specific Pokémon species from the Mega Table.
* - Bit 0: Gengar
* - Bit 1: Gardevoir
* - Bit 2: Ampharos
* - Bit 3: Venusaur
* - etc.
*/
  u8 has_encountered_mega_evolution[8];
};
} // namespace savedata