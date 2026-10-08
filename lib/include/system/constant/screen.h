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
 * @file screen.h
 * @brief The two screens of the console.
 */

#pragma once

#include <types.h>

namespace sys {

/// A screen of the console.
enum class Screen : u8 {
  kTop = 0, ///< The top screen: 400 x 240 pixels.
  kBottom = 1 ///< The bottom screen (touch screen): 320 x 240 pixels.
};

} // namespace sys

using sys::Screen;
