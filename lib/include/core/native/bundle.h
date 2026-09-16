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

#include "core/types.h"

namespace core {

/// A "PC" pack: several resources stored back to back behind an offset
/// table (the game's BinLinker format).
struct Bundle {
  u16 signature; // "PC"
  u16 resource_count;
  u32 resource_offset[];

  u32 GetResourceSize(u32 idx) const {
    u32 safe_idx = (idx >= resource_count) ? 0 : idx;
    return resource_offset[safe_idx + 1] - resource_offset[safe_idx];
  }

  uptr GetResource(u32 idx) {
    u32 safe_idx = (idx >= resource_count) ? 0 : idx;
    return ((uptr)this + resource_offset[safe_idx]);
  }
};

} // namespace core

using core::Bundle;
