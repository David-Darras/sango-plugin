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
 * @file studio_save.h
 * @brief Takes the changes of the save that Sango Studio makes while the
 * game runs.
 *
 * The page Save of Sango Studio writes the save file of the game, and writes
 * the blocks that changed in sdmc:/sango-studio/save/blocks.bin. The plugin
 * looks for this file while the game runs. When the player walks freely in
 * the world, it writes the blocks in the save in memory, moves the player
 * when the file asks for it, and removes the file.
 *
 * The file: the signature "SSAV" (4 bytes), the version 1, 1 when the player
 * moves, the number of blocks, the map, the side that the player looks at,
 * the point (three float), then for each block its number, its size and its
 * bytes, to a multiple of 4. All the numbers have 4 bytes.
 *
 * A file that is there at the start of the game is removed: the game read the
 * save file, which already has these changes.
 */

#pragma once

#include "common.h"

namespace sys {
class File;
} // namespace sys

namespace core {

/// Takes the changes of the save that Sango Studio sends to the running game.
class StudioSave {
  MAKE_SINGLETON(StudioSave)

public:
  /// Watches the folder of Sango Studio. Call it once at the start.
  /// Alpha Sapphire only: the other games do nothing.
  static void Load();

private:
  /// The frames between two looks at the file (about two looks a second).
  static constexpr u32 kPollFrames = 30;

  /// The first values of the file.
  struct Header {
    u32 signature;
    u32 version;
    u32 moves_player;
    u32 block_count;
    u32 map;
    u32 facing;
    f32 x;
    f32 y;
    f32 z;
  };

  static void Update();
  /// Reads the whole file one time, and checks that the save in memory can
  /// take each block. With `write`, writes the blocks in memory too.
  static bool ReadBlocks(const Header& header, bool write);
  /// Writes one block in the save in memory, and in the objects that the game
  /// keeps next to it.
  static void WriteBlock(sys::File* file, u32 block, u8* data, u32 size,
                         s64 skip);
  /// Moves the player to the place of the file.
  static void MovePlayer(const Header& header);

  u32 frame_ = 0;
  bool is_started_ = false;
};

} // namespace core
