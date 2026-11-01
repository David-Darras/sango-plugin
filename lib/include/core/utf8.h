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
 * @file utf8.h
 * @brief Changes a UTF-8 text (the text files) into a UTF-16 text (the texts
 * of the game).
 */

#pragma once

#include "core/types.h"

namespace core {

/**
 * @brief Writes a UTF-8 text as UTF-16, with a 0 at the end. The characters
 *        above 0xFFFF are skipped.
 * @param in The UTF-8 text. It does not need a 0 at the end.
 * @param length The number of bytes of `in`.
 * @param out Receives the UTF-16 text.
 * @param capacity The number of characters of `out`, with the 0.
 * @return The number of characters, without the 0.
 */
u32 Utf8ToUtf16(const c8* in, u32 length, c16* out, u32 capacity);

} // namespace core
