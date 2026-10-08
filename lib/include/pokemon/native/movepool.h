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
 * @file movepool.h
 * @brief The learnset of one species: the moves that it learns when its level increases.
 */

#pragma once

#include "common.h"
#include "core/memory.h"
#include "pokemon/address.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/species.h"

namespace pokemon {
/// The learnset of one species and form.
class Movepool {
  SINGLETON(Movepool)
  /// Loads the learnset of a species and form, and returns it.
  STATIC_INLINE Movepool& GetInstance(SpeciesId species, FormId form) {
    ((void(*)(SpeciesId, FormId))address::kLoadMovepool)(species, form);
    return Object();
  }

  /// Returns the learnset that the game loaded last.
  STATIC_INLINE Movepool& Object() {
    const uptr object = address::kMovepoolPointer != 0 ? READ32(address::kMovepoolPointer) : 0;
    return *(Movepool*)(object != 0 ? object : address::kMovepool);
  }

  /// Returns true when the learnset contains the move.
  INLINE bool Contains(MoveId move) const {
    for (u32 i = 0; i < count; i++) {
      if (entry[i].move == move) {
        return true;
      }
    }
    return false;
  }

public:

  SpeciesId species;
  FormId form;
  u8 _0;

  /// One move of the learnset, and its level.
  struct Entry {
    MoveId move;
    u8 level;
    u8 padding;
  };

  Entry* entry;
  /// The number of moves (26 at most).
  u8 count;
};
} // namespace pokemon
