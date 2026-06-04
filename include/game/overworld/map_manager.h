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
#include "game/manager.h"

namespace game {
class Manager;
} // namespace game

namespace overworld {
class Renderer;

class MapManager {
public:
  STATIC_INLINE MapManager& GetInstance() {
    return game::Manager::GetInstance().GetOverworldMapManager();
  }

  INLINE Renderer& GetRenderer() { return *renderer_; }

  INLINE void* GetAddr() { return &renderer_; }

  INLINE u32& GetMapId() { return current_map_id; }

  INLINE u32& GetNextMapId() { return next_map_id; }

private:
  void* _0[23 + 1];
  game::Manager* game_manager_;
  void* _1[2 + 28 + 1 + 2 + 2];

  u32 current_map_id;
  void* _2;

  u32 next_map_id;
  u32 _3;

  Renderer* renderer_;
  void* encounter_manager_;
};
} // namespace overworld