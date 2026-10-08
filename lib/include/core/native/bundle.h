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
 * @file bundle.h
 * @brief A pack of several resources in one file.
 */

#pragma once

#include "core/types.h"

namespace core {

/// A "PC" pack: several resources one after the other, with a table of
/// offsets at the start. Many files of the game use this format.
struct Bundle {
  u16 signature; // "PC"
  u16 resource_count;
  u32 resource_offset[];

  /// Returns the size of a resource in bytes. A wrong index gives the size of resource 0.
  u32 GetResourceSize(u32 idx) const {
    u32 safe_idx = (idx >= resource_count) ? 0 : idx;
    return resource_offset[safe_idx + 1] - resource_offset[safe_idx];
  }

  /// Returns the address of a resource. A wrong index gives resource 0.
  uptr GetResource(u32 idx) {
    u32 safe_idx = (idx >= resource_count) ? 0 : idx;
    return ((uptr)this + resource_offset[safe_idx]);
  }
};

} // namespace core

using core::Bundle;
