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

#include "core/constant/archive_id.h"
#include "core/types.h"

namespace core {

/// A queued read the game's file manager is about to serve.
struct ArchiveInput {
  u8 priority;
  ArchiveId archive_id;
  u32 file_id;
  bool is_compressed;
  uptr heap[4];
  uptr buffer;
  u32* size;
};

} // namespace core
