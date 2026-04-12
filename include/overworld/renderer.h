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

#ifndef SANGO_PLUGIN_OVERWORLD_RENDERER_H
#define SANGO_PLUGIN_OVERWORLD_RENDERER_H

#include "overworld/map_manager.h"

class GameManager;
class Graphics;

namespace overworld {

class StereoCamera {
 public:
  static void LoadMenu(menu::PluginMenu& menu, void* args);
  static void SetupHooks();

 private:
  void* vtable_;
  u32 _0[7];
  u32 _1[5];
  Mtx34 _2;
  Vec3 _3;
  u32 _4[10];

  void* left_camera_;
  void* right_camera_;
};

class Renderer {
 public:
  static Renderer& GetInstance() {
    return MapManager::GetInstance().GetRenderer();
  }

  StereoCamera& GetStereoCamera() { return *stereo_camera2_; }

 private:
  u32 _0[5];
  GameManager* game_manager_;
  Graphics* graphics_;
  void* _1;
  u32 _2[2];
  void* _3[23];

  u32 _4[3];

  f32 _5[640];
  u32 _6[4];

  StereoCamera* stereo_camera_;
  StereoCamera* stereo_camera2_;
};

}  // namespace overworld

#endif  // SANGO_PLUGIN_OVERWORLD_RENDERER_H
