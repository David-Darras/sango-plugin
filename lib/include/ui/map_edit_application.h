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
 * @file map_edit_application.h
 * @brief A touch editor for the attributes of the tiles of a world.
 */

#pragma once

#include "common.h"
#include "ui/application.h"
#include "ui/widget/button.h"

namespace ui {

/**
 * @brief Shows the tiles around the player on the bottom screen and changes
 *        their attributes with the stylus.
 *
 * The left part of the bottom screen is a map seen from above, one square for
 * each tile. The right part has the buttons. The tools:
 * - Paint: the stylus changes the tiles that it crosses (the brush is a
 *   square of 1 to 9 tiles).
 * - Rect: drag a rectangle, release to change all its tiles.
 * - Fill: changes the connected tiles that have the same attribute as the
 *   tile that you touch (4000 tiles at most).
 * - Pick: copies the attribute of a tile, to paint it with the attribute
 *   Stamp.
 * - Move: drag to scroll the map. The D-pad also scrolls, and L and R change
 *   the zoom.
 *
 * The changes go through overworld::MapEditor: the game uses them at once,
 * the undo cancels them, and the editor writes them to the file of the world.
 * The top screen shows the attributes of the last tile that you touched.
 *
 * The button **Mode** switches to the mode Events. The map shows frames on
 * the signs (cyan), the triggers (magenta), the warps (lime) and the
 * characters (orange for a trainer, blue for another). The tools:
 * - Select: touch an event to select it. The buttons **Field-** and
 *   **Field+** choose a field of the event, **Val-** and **Val+** change it
 *   (the button **Step** gives the size of the change).
 * - Place: touch a tile to add a sign, a trigger or a warp (the button
 *   below the tool gives the kind).
 * - Move: touch a tile to move the selected event there.
 * - Remove: touch an event that you added to remove it.
 * The changes of a character show when the map loads again (**Reload**).
 * **Save** writes the events of the map (see overworld::TriggerEditor).
 */
class MapEditApplication : public Application {
  MAKE_SINGLETON(MapEditApplication)

public:
  /// Opens the editor on the tile of the player.
  static void Open();

  void Update(sys::Controller& controller) override;
  void DrawTop(sys::Graphics& graphics) override;
  void DrawBottom(sys::Graphics& graphics) override;

private:
  enum Tool : u8 { kPaint, kRect, kFill, kPick, kMove, kToolCount };

  enum Mode : u8 { kTiles, kEvents };
  /// The tools of the mode Events.
  enum EventTool : u8 { kSelect, kPlace, kMoveEvent, kRemove, kEventToolCount };
  enum EventKind : u8 {
    kNoEvent,
    kSignEvent,
    kTriggerEvent,
    kWarpEvent,
    kCharacterEvent
  };

  /// What a tool does to the attribute of a tile.
  enum Attribute : u8 {
    kWall,
    kWalkable,
    kWater,
    kWildOn,
    kWildOff,
    kBlockTrainersOn,
    kBlockTrainersOff,
    kDecorationOn,
    kDecorationOff,
    kGround,
    kFootstep,
    kBattleBackground,
    kStamp,
    kAttributeCount
  };

  enum ButtonId : u8 {
    kToolButton,
    kAttributeButton,
    kValueDown,
    kValueUp,
    kSizeDown,
    kSizeUp,
    kZoomOut,
    kZoomIn,
    kUndo,
    kRedo,
    kHome,
    kSave,
    kDone,
    kModeButton,
    kButtonCount
  };

  /// The attribute that the tool gives to a tile whose attribute is `old`.
  u32 Compute(u32 old) const;
  /// Changes one tile. Returns false when the editor is full.
  bool ApplyToTile(s32 x, s32 z);
  /// Changes the tiles under the brush.
  void Stamp(s32 x, s32 z);
  /// Draws the brush on a line, so that a fast stylus leaves no gaps.
  void Stroke(s32 from_x, s32 from_z, s32 to_x, s32 to_z);
  void FillArea(s32 x, s32 z);
  void ApplyRect(s32 x0, s32 z0, s32 x1, s32 z1);
  void Pick(s32 x, s32 z);

  // The mode Events (ui/map_edit_events.cc).
  void Navigate(sys::Controller& controller);
  void UpdateEvents(sys::Controller& controller);
  void DrawEventsTop();
  void DrawEventMarkers(s32 origin_x, s32 origin_z, s32 cell, s32 count);
  bool SelectAt(s32 x, s32 z);
  void MoveSelectedTo(s32 x, s32 z);
  void PlaceAt(s32 x, s32 z);
  void RemoveAt(s32 x, s32 z);
  u32 GetFieldCount() const;
  s32 GetField(u32 field) const;
  void SetField(u32 field, s32 value);
  void ChangeField(s32 delta);
  void ReloadMap();

  bool GetTileAt(s32 touch_x, s32 touch_y, s32& tile_x, s32& tile_z) const;
  void ClampView();
  /// Draws the buttons and the line of information.
  void DrawPanel();
  void GoHome();
  void SetMessage(const c16* text);
  void Close();
  bool IsPressed(ButtonId id) const;
  bool UsesValue() const;
  u32 GetValueMax() const;

  Button buttons_[kButtonCount];
  s32 view_x_ = 0; ///< The tile at the center of the map.
  s32 view_z_ = 0;
  s32 cursor_x_ = 0; ///< The last tile that the stylus touched.
  s32 cursor_z_ = 0;
  Tool tool_ = kPaint;
  Attribute attribute_ = kWall;
  u32 value_ = 0;
  u32 stamp_ = 0;
  bool has_stamp_ = false;
  u8 size_index_ = 0;
  u8 zoom_index_ = 2;
  bool was_down_ = false;
  s32 last_x_ = 0; ///< The last tile of the stroke.
  s32 last_z_ = 0;
  s32 start_x_ = 0; ///< The first tile of a rectangle.
  s32 start_z_ = 0;
  s32 anchor_view_x_ = 0; ///< The view when the move started.
  s32 anchor_view_z_ = 0;
  s32 anchor_touch_x_ = 0;
  s32 anchor_touch_y_ = 0;
  Mode mode_ = kTiles;
  EventTool event_tool_ = kSelect;
  u8 place_kind_ = 0;
  EventKind selected_kind_ = kNoEvent;
  u32 selected_index_ = 0;
  u32 selected_field_ = 0;
  u8 step_index_ = 0;
  bool is_events_used_ = false;
  u32 value_hold_ = 0; ///< The frames while a value button is down.
  bool is_full_ = false;
  const c16* message_ = nullptr;
  u32 message_frames_ = 0;
};

} // namespace ui
