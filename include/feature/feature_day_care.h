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
#include "cheat_code_manager.h"
#include "common.h"

namespace feature {
class DayCare {
  MAKE_SINGLETON(DayCare)
public:
  bool instant_egg_hatch = false;
  bool instant_egg_generation = false;
  bool instant_max_exp = false;

  STATIC_INLINE void PatchLoad() {
    auto& day_care = GetInstance();
    WRITE32(ADDRESS_EGG_HATCH,
            day_care.instant_egg_hatch ? 0xEA000007 : 0x0A000007);
    WRITE32(ADDRESS_EGG_GENERATION,
            day_care.instant_egg_generation ? 0xE1A00000 : 0x2A000002);
    WRITE32(ADDRESS_DAY_CARE_MAX_EXP,
            day_care.instant_max_exp ? 0x15824004 : 0x1582C004);
    WRITE32(ADDRESS_DAY_CARE_MAX_EXP + 0x20,
            day_care.instant_max_exp ? 0x158240F4 : 0x1582C0F4);
  }
};
} // namespace feature