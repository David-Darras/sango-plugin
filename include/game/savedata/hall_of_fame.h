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

#include "game/savedata/savedata.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/species.h"

namespace savedata {
struct HallOfFame {
  SINGLETON(HallOfFame)
  STATIC_INLINE HallOfFame& GetInstance() {
    return SaveData::GetInstance().GetHallOfFame();
  }

  struct Pokemon {
    Species species;
    ItemID item;
    MoveID moves[4];
    u32 id0;
    u32 id1;

    union {
      u32 flags;

      struct {
        u32 form : 5;
        u32 gender : 2;
        u32 level : 7;
        u32 is_shiny : 1;
        u32 has_nickname : 1;
        u32 trainer_gender : 1;
        u32 _0 : 15;
      };
    };

    c16 nickname[12];
    c16 trainer_name[12];
  };

  struct Entry {
    Pokemon pokemon[6];

    union {
      u32 flags;

      struct {
        u32 count : 14;
        u32 year : 8;
        u32 month : 4;
        u32 day : 5;
        u32 is_used : 1;
      };
    };
  };

  void* vtable;
  Entry entries[16];
};
} // namespace savedata