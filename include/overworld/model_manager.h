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

#ifndef SANGO_PLUGIN_OVERWORLD_MODEL_MANAGER_H
#define SANGO_PLUGIN_OVERWORLD_MODEL_MANAGER_H

#include "common.h"
#include "system/game_data_manager.h"

namespace overworld {

struct Position {
  u32 _0;
  Vec3 coords;
  Vec3 _1;
  Vec3 _2;
  u32 _3;
};

struct Model {
  static constexpr u32 kSize = 0xAB0;

  bool IsUsed() const { return (flags[0] & 1) == 1; }

  void* vtable;
  u32 flags[2];

  u16 id;  // 0xFF = player
  u16 map_id;
  u16 model_id;
  u16 move_id;
  u16 _0[4];

  Vec3 facing_direction;
  Vec3 movement_direction;
  Vec3 prev_facing_direction;
  Vec3 prev_movement_direction;

  u16 _2[12];

  Position init_pos;
  Position prev_pos;
  Position map_pos;
  Position world_pos;
  Vec3 draw_pos;
  Vec3 draw_offset;
};

struct ModelResource {
  u32 _0[5];
  u16 model_id;
  u16 padding;
};

class ModelManager {
 public:
  static constexpr u32 kPlayerId = 0xFF;

  static ModelManager& GetInstance() {
    return GameDataManager::GetInstance().GetOverworldModelManager();
  }

  static void LoadMenu(menu::PluginMenu& menu, void* args);
  static void Noclip(void*);
  static void SwarmMod(void*);

  ModelResource& GetResource(u32 idx) { return resources_[idx]; }

  Model& GetPlayer() {
    for (u32 i = 0; i < kMaxModels; i++) {
      Model& model = *(Model*)((uptr)overworld_models_ + Model::kSize * i);
      if (model.id == kPlayerId) {
        return model;
      }
    }
    return overworld_models_[0];
  }

  Model& GetModel(u32 idx) {
    return *(Model*)((uptr)overworld_models_ + Model::kSize * idx);
  }

  static constexpr u32 kMaxModels = 32;

 private:
  void* heap_;
  GameDataManager* game_data_manager_;
  void* _0[5];
  u8 _1[0x3680];
  Model* overworld_models_;
  u32 overworld_model_count_;

  u32 _2[8];

  u32 resource_count_;
  u32 resource_count_2_;
  ModelResource resources_[kMaxModels];
};

}  // namespace overworld

#endif  // SANGO_PLUGIN_OVERWORLD_MODEL_MANAGER_H
