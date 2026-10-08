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
 * @file color.h
 * @brief The color types: Color (decimal values) and Color8 (bytes).
 */

#pragma once

#include "core/types.h"

/// A color with decimal values from 0.0 to 1.0. The default color is white.
struct Color {
  f32 r, g, b, a; ///< Red, green, blue and alpha (opacity).

  Color() : r(1), g(1), b(1), a(1) {}
  Color(f32 r0, f32 g0, f32 b0, f32 a0) : r(r0), g(g0), b(b0), a(a0) {}
};

/// A color with byte values from 0 to 255. The default color is white.
struct Color8 {
  u8 r, g, b, a; ///< Red, green, blue and alpha (opacity).

  Color8() : r(255), g(255), b(255), a(255) {}
  Color8(u8 r0, u8 g0, u8 b0, u8 a0) : r(r0), g(g0), b(b0), a(a0) {}

  /// Returns the four bytes as one 32-bit value.
  INLINE u32 GetRaw() const {
    return *reinterpret_cast<const u32*>(this);
  }
};
