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
#include "game/constant/move_type.h"

/* [TYPE CHART]
 raw = attacking type
 column = defending type
      NOR FIG FLY POI GRO ROC BUG GHO STE FIR WAT GRA ELE PSY ICE DRA DAR FAI
NOR   1.0 1.0 1.0 1.0 1.0 0.5 1.0 0.0 0.5 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0
FIG   2.0 1.0 0.5 0.5 1.0 2.0 0.5 0.0 2.0 1.0 1.0 1.0 1.0 0.5 2.0 1.0 2.0 0.5
FLY   1.0 2.0 1.0 1.0 1.0 0.5 2.0 1.0 0.5 1.0 1.0 2.0 0.5 1.0 1.0 1.0 1.0 1.0
POI   1.0 1.0 1.0 0.5 0.5 0.5 1.0 0.5 0.0 1.0 1.0 2.0 1.0 1.0 1.0 1.0 1.0 2.0
GRO   1.0 1.0 0.0 2.0 1.0 2.0 0.5 1.0 2.0 2.0 1.0 0.5 2.0 1.0 1.0 1.0 1.0 1.0
ROC   1.0 0.5 2.0 1.0 0.5 1.0 2.0 1.0 0.5 2.0 1.0 1.0 1.0 1.0 2.0 1.0 1.0 1.0
BUG   1.0 0.5 0.5 0.5 1.0 1.0 1.0 0.5 0.5 0.5 1.0 2.0 1.0 2.0 1.0 1.0 2.0 0.5
GHO   0.0 1.0 1.0 1.0 1.0 1.0 1.0 2.0 1.0 1.0 1.0 1.0 1.0 2.0 1.0 1.0 0.5 1.0
STE   1.0 1.0 1.0 1.0 1.0 2.0 1.0 1.0 0.5 0.5 0.5 1.0 0.5 1.0 2.0 1.0 1.0 2.0
FIR   1.0 1.0 1.0 1.0 1.0 0.5 2.0 1.0 2.0 0.5 0.5 2.0 1.0 1.0 2.0 0.5 1.0 1.0
WAT   1.0 1.0 1.0 1.0 2.0 2.0 1.0 1.0 1.0 2.0 0.5 0.5 1.0 1.0 1.0 0.5 1.0 1.0
GRA   1.0 1.0 0.5 0.5 2.0 2.0 0.5 1.0 0.5 0.5 2.0 0.5 1.0 1.0 1.0 0.5 1.0 1.0
ELE   1.0 1.0 2.0 1.0 0.0 1.0 1.0 1.0 1.0 1.0 2.0 0.5 0.5 1.0 1.0 0.5 1.0 1.0
PSY   1.0 2.0 1.0 2.0 1.0 1.0 1.0 1.0 0.5 1.0 1.0 1.0 1.0 0.5 1.0 1.0 0.0 1.0
ICE   1.0 1.0 2.0 1.0 2.0 1.0 1.0 1.0 0.5 0.5 0.5 2.0 1.0 1.0 0.5 2.0 1.0 1.0
DRA   1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 0.5 1.0 1.0 1.0 1.0 1.0 1.0 2.0 1.0 0.0
DAR   1.0 0.5 1.0 1.0 1.0 1.0 1.0 2.0 1.0 1.0 1.0 1.0 1.0 2.0 1.0 1.0 0.5 0.5
FAI   1.0 2.0 1.0 0.5 1.0 1.0 1.0 1.0 0.5 0.5 1.0 1.0 1.0 1.0 1.0 2.0 2.0 1.0
*/
namespace feature {
class TypeChart {
  MAKE_SINGLETON(TypeChart)
  enum class Multiplier : u8 {
    k0 = 0,
    k05 = 1,
    k1 = 2,
    k2 = 4
  };

  STATIC_INLINE void PatchLoad() {
    // for (u32 i = 0; i < 18; i++) {
    //   for (u32 j = 0; j < 18; j++) {
    //     MoveType a = static_cast<MoveType>(i);
    //     MoveType d = static_cast<MoveType>(j);
    //     Set(a, d, Multiplier::k0);
    //   }
    // }
  }

  STATIC_INLINE Multiplier (* GetTable())[static_cast<std::size_t>(
    MoveType::kCount)] {
    return reinterpret_cast<Multiplier (*)[static_cast<std::size_t>(
      MoveType::kCount)]>(
      ADDRESS_BATTLE_TYPE_CHART);
  }

  STATIC_INLINE void Set(MoveType attacking_type, MoveType defending_type,
                         Multiplier value) {
    u8 row = static_cast<u8>(attacking_type);
    u8 col = static_cast<u8>(defending_type);
    GetTable()[row][col] = value;
  }

  STATIC_INLINE Multiplier
  Get(MoveType attacking_type, MoveType defending_type) {
    u8 row = static_cast<u8>(attacking_type);
    u8 col = static_cast<u8>(defending_type);
    return GetTable()[row][col];
  }
};
} // namespace feature