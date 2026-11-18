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
 * @file shiny.h
 * @brief Changes the shiny rate.
 */

#pragma once

#include <type_traits>

#include "common.h"
#include "pokemon/constant/shiny_rate.h"

namespace pokemon {

/// The settings of pokemon::Shiny.
struct ShinySettings {
  ShinyRate rate = ShinyRate::kOff; ///< The shiny rate. kOff: the rate of the game.
};
static_assert(std::is_standard_layout<ShinySettings>::value,
              "ShinySettings must have standard layout");

/// Changes the chance that a Pokémon is shiny.
struct Shiny : public ShinySettings {
  MAKE_SINGLETON(Shiny)

  /// A shiny value that is always shiny.
  static constexpr u32 kForcedShiny = 0xFFFFFFFEu;
  /// A shiny value that is never shiny.
  static constexpr u32 kForcedNormal = 0xFFFFFFFFu;

  static void Initialize();

  /// The value of SetThreshold() that keeps the chance of the game.
  static constexpr u32 kGameThreshold = 0xFFFFFFFFu;

  /// Sets the chance of a shiny Pokémon with an exact number: `threshold` out of 65536 (0 never, 65536
  /// always, 16 is the chance of the game: 1 out of 4096). kGameThreshold keeps the chance of the game.
  /// The setting `rate` has priority when it is not kOff. Sango Studio uses it (see
  /// pokemon/patch/studio_formulas.h).
  static void SetThreshold(u32 threshold);

private:
  static u32 ToShinyHook(u32 id, u32 pid);
  static u32 ToNormalHook(u32 id, u32 pid);
  static bool IsShinyHook(u32 id, u32 pid);
};
} // namespace pokemon
