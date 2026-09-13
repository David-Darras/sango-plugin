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

namespace pokemon {

/// Odds of a Pokémon being shiny: 1 chance out of 2^(n-1), kOff leaves the
/// game's own roll untouched.
enum class ShinyRate : u32 {
  kOff,
  k1_1,
  k1_2,
  k1_4,
  k1_8,
  k1_16,
  k1_32,
  k1_64,
  k1_128,
  k1_256,
  k1_512,
  k1_1024,
  k1_2048,
  k1_4096,
  k1_8192,
  k1_16384,
  k1_32768,
  k1_65536,
  k1_131072,
  k1_262144,
  k1_524288,
  k1_1048576,
  kCount,
};

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
