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
#include "game/data_manager.h"

namespace savedata {
struct PssPhoto {
  SINGLETON(PssPhoto)
  STATIC_INLINE PssPhoto& GetInstance() {
    return SaveData::GetInstance().GetPssPhoto();
  }

  void* vtable;
  bool is_used;
  bool _0[3];
  u8 photo[0xC80];
};
} // namespace savedata
// 08C6FBC0