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

#include <types.h>

enum class TextureFormat : u8 {
  kRgba8 = 0,
  kRgb8 = 1,
  kRgba5551 = 2,
  kRgb565 = 3,
  kRgba4 = 4,
  kLa8 = 5,
  kHilo8 = 6,
  kL8 = 7,
  kA8 = 8,
  kLa4 = 9,
  kL4 = 10,
  kA4 = 11,
  kEtc1 = 12,
  kEtc1a4 = 13,
};
