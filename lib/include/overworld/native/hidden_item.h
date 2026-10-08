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
 * @file hidden_item.h
 * @brief The hidden items of the overworld.
 */

#pragma once
#include <3ds/types.h>

#include "core/memory.h"
#include "core/types.h"
#include "overworld/address.h"
#include "overworld/constant/map.h"
#include "pokemon/constant/item.h"

namespace overworld {
/// One hidden item of the game: an item, a map and a position.
struct HiddenItem {
  static constexpr uptr kCount = 171;

  STATIC_INLINE HiddenItem& GetInstance(u32 index) {
    auto* table = (HiddenItem*)READ32(address::kHiddenItemPointer);
    return table[index];
  }

  ItemId item_id;
  u16 uid;
  s16 tile_x;
  s16 height;
  s16 tile_z;
  MapId map_id;

  union {
    u8 flags;

    struct {
      u8 respawn_rate : 7;
      u8 is_initially_placed : 1;
    };
  };
};


/// One random hidden item: one of six items, at a position.
struct RandomHiddenItem {
  static constexpr uptr kCount = 33;

  STATIC_INLINE RandomHiddenItem& GetInstance(u32 index) {
    auto* table = (RandomHiddenItem*)READ32(address::kRandomHiddenItemPointer);
    return table[index];
  }

  MapId map_id;
  union {
    u16 flags;
    struct {
      u16 uid : 15;
      u8 is_mirage_spot : 1;
    };
  };
  s16 tile_x;
  s16 height;
  s16 tile_z;
  ItemId item_id[6];
};
}