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
 * @file map_editor.h
 * @brief Changes the attributes of many tiles of a world, with undo and one
 *        file for each world.
 */

#pragma once

#include "common.h"
#include "overworld/patch/tile_editor.h"

namespace overworld {

/**
 * @brief Changes the attributes of many tiles of a world.
 *
 * A world is the set of tiles that the game keeps for a part of the map of
 * Hoenn (a layout). The tiles have coordinates in this world. The editor keeps
 * the changes as **runs**: a run is a row of tiles that have the same
 * attribute, so a big rectangle costs one run for each row. The game loads
 * the tiles again when the player moves, so the editor writes the runs to the
 * tiles at each third frame.
 *
 * The editor keeps a history of the changed tiles for the undo. A stroke is a
 * group of changes that the undo cancels at once.
 *
 * When the player enters another world, the editor writes the changes of the
 * old world to its file (if auto-save is on), then reads the file of the new
 * world. The files are `sdmc:/sango/maps/layout_<number>.tiles`.
 *
 * The editor can write the tiles around the player to a text file, one
 * character for each tile, and read the file again. With the live reload,
 * it reads the file each time that it changes, so the player can edit the
 * map on a computer. The file is `sdmc:/sango/maps/layout_<number>.txt`.
 *
 * The editor does not know the original attribute of a tile. To get the
 * original tiles again, clear the edits and load the map again.
 */
class MapEditor {
  MAKE_SINGLETON(MapEditor)
public:
  static constexpr u32 kMaxRuns = 1536;
  static constexpr u32 kMaxHistory = 1536;

  /// A row of tiles that have the same attribute.
  struct Run {
    u16 z;
    u16 x;
    u16 length;
    u16 _0;
    u32 attr;
  };

  bool is_enabled = true;
  /// Writes the file when the editor closes or when the world changes.
  bool auto_save = true;
  /// Reads the text file again each time that it changes.
  bool is_live = false;

  /// Called at each frame: changes the world, and writes the runs to the tiles.
  static void Update();

  /// The number of the world that the editor holds, or 0xFFFF.
  static u32 GetLayout();
  static bool IsDirty();
  static u32 GetRunCount();
  static u32 GetTileCount();

  /// The flags that the game gives to a tile with this attribute. The game
  /// looks the bits 0 to 2 (wall, water, wild Pokemon) up in a table, with the
  /// ground number (the top byte) as the index. The other bits come from the
  /// attribute itself.
  static u32 DecodeAttr(u32 raw);

  /// Returns the attribute with the ground number that gives the tile these
  /// flags (bit 0 wall, bit 1 water, bit 2 wild Pokemon). The ground number
  /// changes, so the type of the ground changes too.
  static u32 SetLowFlags(u32 raw, u32 low);

  /// Reads the attribute of a tile. Returns false when the tile is not loaded.
  static bool ReadAttr(s32 tile_x, s32 tile_z, u32& attr);

  /// Starts a group of changes. EndStroke() closes it.
  static void BeginStroke();

  /// Changes a tile. Returns false when the tile is not loaded or the editor
  /// is full (see IsFull()).
  static bool SetTile(s32 tile_x, s32 tile_z, u32 attr);
  static void EndStroke();

  /// The last SetTile() failed because the list of runs was full.
  static bool IsFull();

  static bool CanUndo();
  static bool CanRedo();
  static bool Undo();
  static bool Redo();

  /// Writes the file of the world. Returns false when the file cannot be
  /// written.
  static bool Save();

  /// Writes the 128 x 128 tiles around the player to the text file. The
  /// characters: # wall, ~ water, , wild Pokemon, . plain, ? not loaded.
  static bool ExportText();

  /// Reads the text file and changes the tiles that are different. Returns
  /// false when the file does not exist. `changed` receives the number of
  /// tiles that changed.
  static bool ImportText(u32* changed);

  /// Forgets all the changes of the world and deletes its file. The tiles
  /// keep their new attributes until the game loads the map again.
  static void ClearAll();

private:
  struct Change {
    u16 x;
    u16 z;
    u32 old_attr;
    u32 new_attr;
    u16 stroke;
    u8 had_run;
    u8 _0;
  };

  static constexpr u32 kMagic = 0x3154504D; // "MPT1"

  void RefreshBlocks();
  void ApplyRuns();
  void SwitchLayout(u16 layout);
  bool Load();
  void PollText();
  int FindRun(s32 x, s32 z) const;
  void RemoveTile(s32 x, s32 z);
  void AddTile(s32 x, s32 z, u32 attr);
  void WriteTile(s32 x, s32 z, u32 attr, bool has_run);
  void Push(const Change& change);

  Run runs_[kMaxRuns] = {};
  u32 run_count_ = 0;
  Change history_[kMaxHistory] = {};
  u32 history_count_ = 0;
  u32 history_cursor_ = 0;
  u16 stroke_ = 0;
  bool is_in_stroke_ = false;
  bool is_full_ = false;
  bool is_dirty_ = false;
  u16 loaded_layout_ = 0xFFFF;
  u32 frame_ = 0;
  u32 text_hash_ = 0;
  bool has_text_baseline_ = false;
  u32 blocks_frame_ = 0xFFFFFFFF;
  u32 block_count_ = 0;
  TileEditor::Block blocks_[TileEditor::kMaxBlocks] = {};
};

} // namespace overworld
