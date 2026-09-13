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

#include "savedata/native/savedata.h"

namespace savedata {
/// The repel in effect (Repel/Super Repel/Max Repel) and its remaining steps.
struct Repel {
  SINGLETON(Repel)

  STATIC_INLINE Repel& GetInstance() {
    return SaveData::GetInstance().GetRepel();
  }

  void* vtable;
  u16 spray_id;
  u8 spray_count;
};
} // namespace savedata