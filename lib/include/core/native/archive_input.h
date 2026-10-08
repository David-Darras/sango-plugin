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
 * @file archive_input.h
 * @brief A request to read one file of an archive.
 */

#pragma once

#include "core/constant/archive_id.h"
#include "core/types.h"

namespace core {

/// A read request that the file manager of the game is about to run.
/// core::Archive::on_read_file can change it.
struct ArchiveInput {
  u8 priority; ///< The priority of the request.
  ArchiveId archive_id; ///< The archive.
  u32 file_id; ///< The file in the archive. Change it to load a different file.
  bool is_compressed; ///< true when the file is compressed.
  uptr heap[4];
  uptr buffer; ///< The address that receives the address of the loaded data.
  u32* size; ///< Receives the size of the loaded data.
};

} // namespace core
