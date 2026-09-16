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

#include <types.h>

#include "core/bitmask.h"

namespace sys {

enum class Key : u32 {
  kNone = 0,
  kLeft = 1 << 0,
  kRight = 1 << 1,
  kUp = 1 << 2,
  kDown = 1 << 3,
  kA = 1 << 4,
  kB = 1 << 5,
  kX = 1 << 6,
  kY = 1 << 7,
  kL = 1 << 8,
  kR = 1 << 9,
  kZl = 1 << 10,
  kZr = 1 << 11,
  kStart = 1 << 12,
  kSelect = 1 << 13,
  kHome = 1 << 14,
};
ENABLE_BITMASK_OPERATORS(Key)

} // namespace sys

using sys::Key;
