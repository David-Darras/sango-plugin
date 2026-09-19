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

#include "core/native/game_manager.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"
#include "overworld/native/position.h"

namespace core {
class GameManager;
} // namespace core

namespace overworld {
class Renderer;
struct PropModelManager;
struct WorldLayout;

class MapManager {
  SINGLETON(MapManager)

public:
  static constexpr u32 kNoMap = 0xFFFF;

  STATIC_INLINE MapManager& GetInstance() {
    return core::GameManager::GetInstance().GetOverworldMapManager();
  }

  STATIC_INLINE void ChangeMap(MapId map_id, const Position& position,
                               Facing facing, bool keep_background_music,
                               bool show_map_name) {
#ifdef GAME_XY
    ((void (*)(core::GameManager*, MapId, const Vec3*, Facing, u32, bool,
               s32, s32))address::kChangeMap)(
        &core::GameManager::GetInstance(), map_id, &position.coords, facing, 0,
        keep_background_music, 1, 1);
    (void)show_map_name;
#else
    ((void (*)(core::GameManager*, MapId, const Position*, Facing, u8, bool, s32,
               s32, s32, bool))address::kChangeMap)(
        &core::GameManager::GetInstance(), map_id, &position, facing, 0,
        keep_background_music, 1, 1, 1, show_map_name);
#endif
  }

  INLINE Renderer* GetRenderer() const { return renderer_; }

  INLINE void* GetAddr() { return &renderer_; }

  INLINE u32& GetMapId() { return current_map_id; }
  INLINE MapId GetMap() const { return static_cast<MapId>(current_map_id); }

  INLINE u32& GetNextMapId() { return next_map_id; }

  INLINE void* GetHeap() const { return heap_; }
  INLINE void* GetDeviceHeap() const { return device_heap_; }
  INLINE void* GetHeapAllocator() const { return heap_allocator_; }
  INLINE void* GetDeviceAllocator() const {
    return device_heap_allocator_;
  }

  INLINE void* GetMapper() const { return mapper_; }

  INLINE PropModelManager& GetPropModelManager() const { return *prop_model_manager_; }
  INLINE WorldLayout& GetWorldLayout() const { return *world_layout_; }

private:
  void* __0; // 0x00
  void* heap_; // 0x04
  void* __1; // 0x08
  void* __2; // 0x0C
  void* device_heap_; // 0x10
  void* __3; // 0x14
  void* __4; // 0x18
  void* __5; // 0x1C
  void* __6; // 0x20
  void* heap_allocator_; // 0x24
  void* __7; // 0x28
  void* device_heap_allocator_; // 0x2C
  void* __8; // 0x30
  void* __9; // 0x34
  void* __10; // 0x38
  void* _0[GAME_CONSTANT(4, 8)]; // 0x3C
  u32 _1; // 0x5C / 0x4C
  core::GameManager* game_manager_; // 0x60 / 0x50
  WorldLayout* world_layout_; // 0x64 / 0x54
  void* _2[34]; // 0x68 / 0x58

  u32 current_map_id; // 0xF0 / 0xE0
  void* map_data_; // 0xF4 / 0xE4

  u32 next_map_id; // 0xF8 / 0xE8
  u32 next_map_step; // 0xFC / 0xEC 0 loading, 1 swapping models, 2 setting up

  Renderer* renderer_; // 0x100 / 0xF0
  void* encounter_manager_; // 0x104 / 0xF4
  void* day_care; // 0x108 / 0xF8
  void* mapper_; // 0x10C / 0xFC
  u32 _3[12]; // 0x110 / 0x100
  u32 _4[GAME_CONSTANT(14, 12)]; // 0x140 / 0x130
  PropModelManager* prop_model_manager_; // 0x170 / 0x168
};
} // namespace overworld
