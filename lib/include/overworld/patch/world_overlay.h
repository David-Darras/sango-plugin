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
 *        player, the zones of the events, the characters and the hidden items.
 */

#pragma once

#include "common.h"

namespace overworld {
/**
 * @brief Draws marks on the overworld, on the top screen (ORAS only).
 *
 * The overlay keeps the camera that draws the overworld
 * (renderer::address::kSceneDraw). It projects the points of the world on the
 * screen with the matrices of this camera, then draws with sys::Graphics.
 *
 * A tile is a quadrilateral in perspective. The game library draws only
 * rectangles that stay parallel to the screen, so the overlay draws a tile as
 * a pile of thin horizontal strips, and joins the tiles of the same color in
 * a row to draw fewer strips.
 *
 * The layers:
 * - the tiles: red = wall, blue = water, green = wild Pokémon, orange = the
 *   tile blocks trainers, purple = a decoration can stand on it;
 * - the zones of the events: cyan = sign, magenta = trigger that starts when
 *   the player walks in, pink = trigger that starts when the player pushes,
 *   lime = warp;
 * - the characters: a square, with the number of the script;
 * - the line of sight of the trainers: orange tiles in front of the trainer;
 * - a yellow mark shows a hidden item, a white mark the player.
 *
 * The overlay draws nothing while the menu is open: the drawings of the
 * menu and of the overlay together can fill the command list of the GPU.
 */
class WorldOverlay {
  MAKE_SINGLETON(WorldOverlay)
public:
  /// The largest radius: (2 x 8 + 1)^2 = 289 tiles at most.
  static constexpr u32 kMaxRadius = 8;

  bool is_enabled = false;

  // The tiles.
  bool show_tiles = true; ///< Walls, water and wild Pokémon.
  bool show_trainer_blockers = false;
  bool show_decoration_spots = false;
  /// true: the tiles are filled in perspective. false: a small square on each
  /// tile (it needs fewer drawings).
  bool fill_tiles = true;

  // The events.
  bool show_signs = true;
  bool show_triggers = true;
  bool show_warps = true;
  bool show_hidden_items = true;

  // The characters.
  bool show_characters = true;
  bool show_trainer_sight = true;
  bool show_script_numbers = true; ///< Writes the script of the zones.

  bool show_player = true; ///< A mark on the player, to check the camera.
  bool show_legend = false; ///< A list of the colors at the bottom left.

  u8 radius = 5; ///< The tiles around the player (1 to kMaxRadius).
  u8 opacity = 80; ///< The opacity of the colors, in percent (10 to 100).
  /// 0: the image turns by 90 degrees for the screen of the 3DS.
  /// 1: no turn (the value that works with the camera of the game).
  /// Change it when the marks are not at their places.
  u8 pivot = 1;

  /// The state of the last frames, for the page of the menu: the frames where
  /// the overlay had a camera, and the drawings of the last frame.
  u32 camera_frames = 0;
  u32 marks_drawn = 0;
  u32 strips_drawn = 0;
  /// For the page of the menu, to find why the overlay shows nothing: the
  /// scene draws that the hooks saw, the cameras of the top screen that the
  /// overlay found, and the cameras that it refused.
  u32 draws_seen = 0;
  u32 cameras_found = 0;
  u32 cameras_refused = 0;

  static void Initialize();

  /// Draws the marks. plugin::DrawFrame() calls it for the top screen.
  static void DrawTop();

private:
  struct ScreenPoint {
    f32 x;
    f32 y;
  };

  /// A group of tiles of the same color in a row, or a zone, in perspective.
  struct Quad {
    ScreenPoint corner[4];
    Color color;
  };

  static constexpr u32 kMaxQuads = 420;

  static void DrawSceneHook(void* scene, void* graphics, void* camera,
                            void* target, s32 index, bool flag);
  static void StereoDrawHook(void* scene, void* graphics, void* camera,
                             bool flag);

  /// Reads the matrices of a camera. Keeps them, and returns true, when the
  /// camera draws the top screen (the shape of its projection tells it).
  bool TryUseCamera(void* camera);

  /// Converts a point of the world into a point of the top screen. Returns
  /// false when the point is behind the camera. The point can be out of the
  /// screen.
  bool ProjectPoint(const Vec3& world, ScreenPoint& out) const;

  /// Converts a point of the world into a pixel of the top screen. Returns
  /// false when the point is behind the camera or out of the screen.
  bool Project(const Vec3& world, s32& x, s32& y) const;

  /// Adds the rectangle of tiles [x, x + width) x [z, z + depth) to the list
  /// of quads. Returns false when the quad is behind the camera, or the list
  /// is full.
  bool AddTileQuad(s32 x, s32 z, s32 width, s32 depth, f32 ground, Color color);

  /// Draws the quads as strips. The thickness of the strips grows when the
  /// quads are big, so that the number of drawings stays small.
  void FlushQuads();

  // The layers that add quads: the tiles, the zones of the events, and the
  // line of sight of the trainers.
  void DrawTileLayer(s32 player_x, s32 player_z, f32 ground);
  void DrawEventLayer(s32 player_x, s32 player_z, f32 ground);
  void AddSightQuads(f32 ground);

  // The marks and the texts: they come after the quads.
  void DrawCharacterMarks(f32 ground);
  void DrawEventLabels(f32 ground);
  /// Writes a number (a script) over a point of the world.
  void DrawLabel(const Vec3& world, u32 number, Color color);
  void DrawLegend();

  bool has_camera_ = false; ///< The matrices below are those of this frame.
  Mtx34 view_ = {};
  Mtx44 projection_ = {};
  Quad quads_[kMaxQuads];
  u32 quad_count_ = 0;
  u32 label_count_ = 0;
};
} // namespace overworld
