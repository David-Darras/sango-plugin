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
 * @file uid.h
 * @brief The battle id of a Pokémon.
 */

#pragma once

#include <types.h>

namespace battle {

/// The battle id of a Pokémon: client * 6 + party slot.
struct UID {
  static constexpr u8 kNoneValue = 0xFF; ///< The value that means "no Pokémon".

  u8 value;

  static constexpr UID None() { return UID{kNoneValue}; }
  constexpr bool IsNone() const { return value == kNoneValue; }

  constexpr bool operator==(UID other) const {
    return value == other.value;
  }
};

} // namespace battle
