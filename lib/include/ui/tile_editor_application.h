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

#ifndef SANGO_PLUGIN_TILE_EDITOR_APPLICATION_H
#define SANGO_PLUGIN_TILE_EDITOR_APPLICATION_H
#include "application.h"
#include "common.h"
#include "feature/overworld/feature_tile_editor.h"

class Controller;
class Graphics;

namespace ui {
class TileEditorApplication : public Application {
  MAKE_SINGLETON(TileEditorApplication)

public:
  static constexpr s32 kTileSize = 8;
  static constexpr s32 kColumns = 320 / kTileSize;
  static constexpr s32 kRows = 240 / kTileSize;

  static void Open();

  void Update(Controller& controller) override;
  void DrawTop(Graphics& graphics) override;
  void DrawBottom(Graphics& graphics) override;

private:
  enum Property : u32 {
    kGround,
    kFootstep,
    kBattleBackground,
    kDiagonal,
    // kShadow,
    // kTrainerBlocked,
    // kDecorations,
    // kDecorationCursor,
    // kReflection0,
    // kReflection1,
    // kReflection2,
    // kReflection3,
    kPropertyCount
  };

  void Adjust(s32 delta);
  void Paint();
  Color ColorOf(const overworld::Tile& tile) const;

  s32 cursor_x_ = 0;
  s32 cursor_z_ = 0;
  u32 property_ = kGround;
  u32 brush_ = 0;
  bool has_brush_ = false;
  bool saved_ = false;
};
} // namespace ui

#endif //SANGO_PLUGIN_TILE_EDITOR_APPLICATION_H
