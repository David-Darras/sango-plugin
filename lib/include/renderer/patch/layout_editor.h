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
 * @file layout_editor.h
 * @brief Finds the panes of the 2D layouts of the screen, and keeps the
 *        changes that the player makes to them.
 */

#pragma once

#include "common.h"
#include "core/color.h"
#include "renderer/native/pane.h"

namespace renderer {

/// The kind of a pane (the class of its object).
enum class PaneKind : u8 {
  kPane, ///< A pane without a picture: a group.
  kTextBox,
  kPicture,
  kWindow,
  kBounding, ///< The touch area of a button.
  kUnknown,
};

/// The values of a pane that the menu changes.
struct PaneState {
  f32 translate[3] = {0, 0, 0};
  f32 rotate[3] = {0, 0, 0};
  f32 scale[2] = {1, 1};
  f32 size[2] = {0, 0};
  u8 alpha = 255;
  bool is_visible = true;
  u8 color_count = 0; ///< 2 for a text box, 4 for a picture, else 0.
  u8 _0 = 0;
  Color8 colors[4];
};

/**
 * @brief The panes of the 2D layouts (the menus of the game) of the current
 *        screen.
 *
 * A layout is a tree of panes: a root pane, its children, the children of the
 * children... The game calculates the matrices of all the panes one time for
 * each frame, after its animations and before the drawing. The editor hooks
 * this calculation. It keeps the root panes that it sees, so that the menu can
 * show the tree. It also writes the changes of the player into a pane before
 * the game calculates it: the animations of the game cannot undo them.
 *
 * A change is for a pane that has a path ("RootPane@400/N_Menu/T_Title") in a
 * screen (the process of the game). The file sdmc:/sango/layouts.ini keeps
 * the changes, and the editor reads it at the start.
 */
class LayoutEditor {
  MAKE_SINGLETON(LayoutEditor)
public:
  static constexpr u32 kMaxRoots = 12;
  static constexpr u32 kMaxEdits = 24;
  static constexpr u32 kMaxPathLength = 96;

  /// Installs the hook and reads the saved changes. ORAS only.
  static void Initialize();

  /// Call it one time for each frame. It forgets the roots of the old frames
  /// and reads the name of the current screen.
  static void Update();

  /// Returns the kind of a pane.
  static PaneKind GetKind(const Pane* pane);

  /// Writes the root panes of the screen. Returns their number.
  static u32 GetRoots(Pane** out, u32 max);

  /// Returns true when `pane` is in one of the trees of the screen. The menu
  /// uses it before it reads a pane that it kept: the pane can be gone.
  static bool IsAlive(const Pane* pane);

  /// Returns the number of panes in the tree of a pane (this pane included).
  static u32 CountPanes(const Pane* pane);

  /// Returns the n-th child of a pane, or null.
  static Pane* GetChild(const Pane* pane, u32 index);

  /// Returns the number of children of a pane.
  static u32 GetChildCount(const Pane* pane);

  /// Writes the path of a pane: the names from the root, with a slash.
  static void GetPath(const Pane* pane, c8* out, u32 size);

  /// Reads the values of a pane.
  static PaneState Read(const Pane* pane);

  /// Writes the values to a pane. The game calculates its matrix again.
  static void Write(Pane* pane, const PaneState& state);

  /// Keeps a change of a pane: the game gets it again at each frame.
  /// `original` is the state before the first change. Returns false when
  /// the list is full.
  static bool SetEdit(const Pane* pane, const PaneState& original,
                      const PaneState& state);

  /// Returns the change of a pane, or null.
  static const PaneState* GetEdit(const Pane* pane);

  /// Returns the state of a pane before its first change, or null.
  static const PaneState* GetOriginal(const Pane* pane);

  /// Removes the change of a pane, and writes its original values.
  static void RemoveEdit(Pane* pane);

  /// Removes all the changes of the current screen.
  static void RemoveAllEdits();

  /// The number of changes of all the screens.
  static u32 GetEditCount();

  /// Writes the changes to sdmc:/sango/layouts.ini. Returns true on success.
  static bool Save();

  /// The name of the current screen (the class of its process).
  static const c8* GetScreenName();

private:
  struct Root {
    Pane* pane;
    u32 frame;
    /// The width of the root pane when the editor saw it first: 400 for the
    /// top screen, 320 for the bottom screen. It tells two layouts of one
    /// screen apart when their panes have the same names.
    u32 tag;
  };

  struct Edit {
    bool is_used;
    u32 screen_hash;
    c8 screen[48];
    c8 path[kMaxPathLength];
    PaneState original;
    PaneState state;
  };

  static void UpdateMatrixHook(Pane* pane, const void* draw_info);
  void Register(Pane* pane);
  void Apply(Pane* pane);
  static bool Load();
  Edit* Find(const Pane* pane);
  /// Writes the original values to all the panes that this change touches.
  static void Restore(const Edit& edit);

  Root roots_[kMaxRoots] = {};
  Edit edits_[kMaxEdits] = {};
  u32 edit_count_ = 0;
  u32 frame_ = 0;
  u32 screen_hash_ = 0;
  c8 screen_[48] = "";
};

} // namespace renderer
