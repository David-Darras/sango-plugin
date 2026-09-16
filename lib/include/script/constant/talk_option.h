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

/// Options of the game's TalkMdlStart / TalkMdlEnd natives.
enum class TalkOption : u16 {
  kNone = 0,
  kNoZoom = 1 << 0,
  kNoHalfSit = 1 << 1,
  kNoEyeContact = 1 << 2,
  kNoTurn = 1 << 3,
  kMotion = 1 << 4,
  kNoSound = 1 << 5,
  kNoKneel = 1 << 6,
};
ENABLE_BITMASK_OPERATORS(TalkOption)

} // namespace script
