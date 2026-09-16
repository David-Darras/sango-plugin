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
#include "pokemon/constant/move.h"
#include "pokemon/constant/species.h"

namespace pokemon {
class Movepool {
  SINGLETON(Movepool)
  STATIC_INLINE Movepool& GetInstance(SpeciesId species, FormId form) {
    ((void(*)(SpeciesId, FormId))address::kLoadMovepool)(species, form);
    return *(Movepool*)address::kMovepool;
  }

  INLINE bool Contains(MoveId move) const {
    for (u32 i = 0; i < count; i++) {
      if (entry[i].move == move) {
        return true;
      }
    }
    return false;
  }

public:
  // 0014EB80

  SpeciesId species;
  FormId form;
  u8 _0;

  struct Entry {
    MoveId move;
    u8 level;
    u8 padding;
  };

  Entry* entry;
  // MAX = 26
  u8 count;
};
} // namespace pokemon
