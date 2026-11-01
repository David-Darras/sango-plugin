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
 * @file parse_float.h
 * @brief Reads a decimal number of a text file, like strtof().
 *
 * strtof() of the C library is big: it reads all the forms of the numbers
 * (hexadecimal, infinity...). The files of the editors write only "-1.250"
 * or "3e-2": this function reads these forms, and is much smaller.
 */

#pragma once

#include "core/types.h"

namespace core {

/**
 * @brief Reads a number after the spaces: an optional sign, digits, an
 *        optional point and digits, then an optional exponent (e-2).
 * @param text The text.
 * @param end Receives the address after the number, or `text` when there is
 *        no number. Can be null.
 * @return The number, or 0 when there is no number.
 */
f32 ParseFloat(const c8* text, c8** end);

} // namespace core
