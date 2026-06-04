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

#include "core/core.h"
#include "game/manager.h"

namespace global_data {
struct Item {
  INLINE Item(const u16 id) {
    ((void(*)(Item*, u16, void*))ADDRESS_GLOBAL_DATA_ITEM_INITIALIZE)
        (this, id, game::Manager::GetInstance().GetSystemHeap());
  }

  INLINE void GetName(String* str) {
    ((void(*)(Item*, String*, void*))ADDRESS_GLOBAL_DATA_ITEM_GET_NAME)
        (this, str, game::Manager::GetInstance().GetSystemHeap());
  }

  u16 price;
  u8 _0[34];
  u32 id;
};
} // namespace global_data