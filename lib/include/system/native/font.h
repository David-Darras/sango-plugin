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
 * @file font.h
 * @brief A font of the game.
 */

#pragma once

#include "common.h"

namespace sys {

/// A font of the game.
struct Font {
  /// Returns true when the font can draw the character.
  INLINE bool HasGlyph(u16 c) {
    return ((bool(*)(Font*, u16))ui::address::kFontHasGlyph)(this, c);
  }
};

} // namespace sys
