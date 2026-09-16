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

#include <type_traits>

#include "common.h"
#include "pokemon/constant/shiny_rate.h"

namespace pokemon {

struct ShinySettings {
  ShinyRate rate = ShinyRate::kOff;
};
static_assert(std::is_standard_layout<ShinySettings>::value,
              "ShinySettings must have standard layout");

struct Shiny : public ShinySettings {
  MAKE_SINGLETON(Shiny)

  static constexpr u32 kForcedShiny = 0xFFFFFFFEu;
  static constexpr u32 kForcedNormal = 0xFFFFFFFFu;

  static void Initialize();

private:
  static u32 ToShinyHook(u32 id, u32 pid);
  static u32 ToNormalHook(u32 id, u32 pid);
  static bool IsShinyHook(u32 id, u32 pid);
};
} // namespace pokemon
