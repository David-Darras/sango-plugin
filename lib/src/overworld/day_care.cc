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

#include "overworld/patch/day_care.h"

namespace overworld {
namespace {
constexpr u32 kMaxExpOriginal0 = GAME_CONSTANT(0x15803004, 0x1582C004);
constexpr u32 kMaxExpInstant0 = GAME_CONSTANT(0x15802004, 0x15824004);
constexpr u32 kMaxExpOriginal1 = GAME_CONSTANT(0x158010F4, 0x1582C0F4);
constexpr u32 kMaxExpInstant1 = GAME_CONSTANT(0x158020F4, 0x158240F4);
} // namespace

void DayCare::PatchLoad() {
  auto& day_care = GetInstance();
  if (pokemon::address::kEggHatch) {
    WRITE32(pokemon::address::kEggHatch,
            day_care.instant_egg_hatch ? 0xEA000007 : 0x0A000007);
  }
  if (pokemon::address::kDayCareMaxExp) {
    WRITE32(pokemon::address::kDayCareMaxExp,
            day_care.instant_max_exp ? kMaxExpInstant0 : kMaxExpOriginal0);
    WRITE32(pokemon::address::kDayCareMaxExp + 0x20,
            day_care.instant_max_exp ? kMaxExpInstant1 : kMaxExpOriginal1);
  }
}

void DayCare::ApplyEggHatch(void*) {
  SAFE_WRITE32(
      pokemon::address::kEggHatch,
      GetInstance().instant_egg_hatch ? 0xEA000007 : 0x0A000007
  );
}

void DayCare::ApplyMaxExp(void*) {
  auto& day_care = GetInstance();
  SAFE_WRITE32(pokemon::address::kDayCareMaxExp,
               day_care.instant_max_exp ? kMaxExpInstant0 : kMaxExpOriginal0);
  SAFE_WRITE32(pokemon::address::kDayCareMaxExp + 0x20,
               day_care.instant_max_exp ? kMaxExpInstant1 : kMaxExpOriginal1);
}

} // namespace overworld
