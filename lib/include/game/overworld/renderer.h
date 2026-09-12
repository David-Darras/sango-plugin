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

#pragma once

#include "common.h"
#include "game/overworld/map_manager.h"
#include "game/renderer/h3d_resource.h"

namespace game {
class Manager;
} // namespace game

namespace overworld {
struct StereoCamera {
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
  SINGLETON(Renderer)

public:
  STATIC_INLINE Renderer* GetInstance() {
    return MapManager::GetInstance().GetRenderer();
  }

  INLINE void* GetScene() const { return scene_; }

  INLINE renderer::H3dResource* GetShaderResource() const {
    return (renderer::H3dResource*)shader_h3d_resource_;
  }

  INLINE void* GetHeapAllocator() const { return heap_allocator_; }
  INLINE void* GetDeviceAllocator() const { return device_allocator_; }

  INLINE StereoCamera* GetStereoCamera() const { return use_camera_; }

private:
  u32 _000[5]; // 0x00 m_animation_camera
  game::Manager* game_manager_; // 0x14
  void* _00; // 0x18
  void* _0; // 0x1C
  u32 _1; // 0x20
  s32 _2; // 0x24
  void* heap_allocator_; // 0x28
  void* device_allocator_; // 0x2C
  void* _3; // 0x30
  void* scene_; // 0x34
  void* _4[4]; // 0x38
  void* _5[2]; // 0x48
  void* _6[2]; // 0x50
  void* _7; // 0x58
  void* shader_h3d_resource_; // 0x5C
  void* _8[8]; // 0x60
  void* _9; // 0x80
  s32 _10; // 0x84
  s32 _11; // 0x88
  s32 _12; // 0x8C
  f32 _13[640]; // 0x90
  u32 _14[4]; // 0xA90
  u32 _15; // 0xAA0
  StereoCamera* use_camera_; // 0xAA4
};
} // namespace overworld
