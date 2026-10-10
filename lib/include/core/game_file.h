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
 * @file game_file.h
 * @brief Reads the files of the archives of the game into the memory of the
 *        plugin, in all the parts of the game (overworld, battle...).
 */

#pragma once

#include "common.h"
#include "core/constant/archive_id.h"

namespace core {
/**
 * @brief Reads the files of the archives of the game, and gives memory of
 *        the heaps that exist for all the game.
 *
 * The heaps of the overworld do not exist in all the parts of the game (for
 * example in a battle). This class uses the resident heaps of the game,
 * and keeps a part of their memory free for the game: a failed allocation
 * of the game stops the game.
 *
 * @code
 * u32 size = 0;
 * u8* file = core::GameFile::Read(ArchiveId::kItemIcon, 4, true, &size);
 * // Use the file, then:
 * delete[] file;
 * @endcode
 */
class GameFile {
public:
  /**
   * @brief Reads a file of an archive into the memory of the plugin.
   * @param archive The archive.
   * @param file_id The file in the archive (the language of the game for an
   *        archive with languages).
   * @param compressed true for a file with LZ compression: the function
   *        gives the file after decompression.
   * @param out_size Receives the size of the file, or null.
   * @return The file (release it with delete[]), or null.
   */
  static u8* Read(ArchiveId archive, u32 file_id, bool compressed,
                  u32* out_size = nullptr);

  /**
   * @brief Gives memory for the GPU: new linear memory of the system, else
   *        a resident heap of the game. The memory stays: the plugin does
   *        not release it.
   * @param size The size in bytes.
   * @param alignment The alignment of the address.
   * @return The memory (an address in the linear memory), or null when no
   *         heap has enough free memory.
   */
  static void* AllocateDevice(u32 size, u32 alignment);
};
} // namespace core
