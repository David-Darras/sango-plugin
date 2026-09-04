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
#include "game/constant/species.h"

namespace global_data {
class Movepool {
  SINGLETON(Movepool)
  STATIC_INLINE Movepool& GetInstance(Species species, Form form) {
    ((void(*)(Species, Form))ADDRESS_GLOBAL_DATA_LOAD_MOVEPOOL)(species, form);
    return *(Movepool*)ADDRESS_GLOBAL_DATA_MOVEPOOL;
  }

  INLINE bool contains(u16 move) {
    for (u32 i = 0; i < count; i++) {
      if (entry[i].move == move) {
        return true;
      }
    }
    return false;
  }

public:
  // 0014EB80

  u16 species;
  u16 form;

  struct Entry {
    u16 move;
    u8 level;
    u8 padding;
  };

  Entry* entry;
  // MAX = 26
  u8 count;
};
}