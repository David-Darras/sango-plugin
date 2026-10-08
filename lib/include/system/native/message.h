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
 * @file message.h
 * @brief A text file of the game.
 */

#pragma once

#include "core/types.h"
#include "system/address.h"
#include "system/constant/language.h"
#include "system/native/string.h"

namespace sys {

/**
 * @brief A text file of the game: for example the names of the moves.
 *
 * Each text has an index (`str_id`) in the file.
 */
struct Message {
  uptr vtable;
  uptr heap;
  u32* archive;
  u8 kind; ///< 0: one text is loaded. 1: all the texts are loaded.
  u32 file_id; ///< The number of the text file.
  Language language; ///< The language of the texts.
  u8 padding[3];
  uptr _0, _1, _2;

  /// Copies the text with the index `str_id` into `output`.
  INLINE void GetString(u32 str_id, String* output) {
    return ((void(*)(Message*, u32, String*))
      sys::address::kMessageGetString)(this, str_id, output);
  }
};

} // namespace sys

using sys::Message;
