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

#include "core/bitmask.h"

namespace script {

/// Options of the game's message window natives (TalkMdlMsg_Seq...).
enum class MessageOption : u32 {
  kNone = 0,
  kCenter = 1 << 0,
  kDoubleSize = 1 << 1,
  kInstant = 1 << 2,
  kPositionUpLeft = 1 << 3,
  kPositionUpRight = 1 << 4,
  kPositionUpCenter = 1 << 5,
  kPositionDownLeft = 1 << 6,
  kPositionDownRight = 1 << 7,
  kPositionDownCenter = 1 << 8,
  kNoTail = 1 << 9,
  kAlignRight = 1 << 14,
};
ENABLE_BITMASK_OPERATORS(MessageOption)

} // namespace script
