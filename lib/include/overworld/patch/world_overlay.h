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
 * @file world_overlay.h
 * @brief Marks on the 3D world of the overworld: the tiles around the
 *        player and the hidden items.
 */

#pragma once

#include "common.h"

namespace overworld {
/**
 * @brief Draws marks on the overworld, on the top screen (ORAS only).
 *
 * The overlay keeps the camera that draws the overworld
 * (gfl::grp::g3d::Scene::Draw). It projects the points of the world on the
 * screen with the matrices of this camera, then draws small rectangles with
 * sys::Graphics. The colors of the tiles: red = wall, blue = water,
 * green = wild Pokémon. A yellow mark shows a hidden item, a white mark the
 * player.
 *
 * The overlay draws nothing while the menu is open: the drawings of the
 * menu and of the overlay together can fill the command list of the GPU.
 */
class WorldOverlay {
  MAKE_SINGLETON(WorldOverlay)
public:
  /// The largest radius: (2 x 4 + 1)^2 = 81 tiles at most.
  static constexpr u32 kMaxRadius = 4;

  bool is_enabled = false;
  bool show_tiles = true;
  bool show_hidden_items = true;
  bool show_player = true; ///< A mark on the player, to check the camera.
  u8 radius = 3; ///< The tiles around the player (1 to kMaxRadius).
  /// 0: the screen of the 3DS turns by 90 degrees (the normal case).
  /// 1: no turn. Change it when the marks are not at their places.
  u8 pivot = 0;

  static void Initialize();

  /// Draws the marks. plugin::DrawFrame() calls it for the top screen.
  static void DrawTop();

private:
  static void DrawSceneHook(void* scene, void* graphics, u32 display,
                            void* camera, u32 command_cache_dump);

  /// Converts a point of the world into a pixel of the top screen. Returns
  /// false when the point is behind the camera or out of the screen.
  bool Project(const Vec3& world, s32& x, s32& y) const;

  void* camera_ = nullptr; ///< The camera of the last frame, or null.
  Mtx34 view_ = {};
  Mtx44 projection_ = {};
};
} // namespace overworld
