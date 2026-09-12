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
#include "game/core/data_manager.h"
#include "game/renderer/h3d_resource.h"

namespace overworld {
struct Position {
  u32 _0;
  Vec3 coords;
  Vec3 _1;
  Vec3 _2;
  u32 _3;
};

struct DrawModel {
  u8 _0[0x150];
  Vec3 position;
  Vec3 rotation;
  Vec3 scale;
};

struct Model {
  static constexpr u32 kSize = 0xAB0;

  INLINE DrawModel& GetDrawModel() {
    return *((DrawModel * (*)(Model*))ADDRESS_OVERWORLD_GET_DRAW_MODEL)(this);
  }

  INLINE bool IsUsed() const { return (flags[0] & 1) == 1; }

  void* vtable;
  u32 flags[2];

  u16 id; // 0xFF = player
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
  u16 code;
  u8 draw_type;
  u8 draw_code;
  u32 _0[4];
  u16 model_id;
  u16 padding;
};

class ModelManager {
  SINGLETON(ModelManager)

public:
  static constexpr u32 kPlayerId = 0xFF;

  STATIC_INLINE ModelManager& GetInstance() {
    return game::DataManager::GetInstance().GetOverworldModelManager();
  }

  INLINE ModelResource& GetResource(u32 idx) { return resources_[idx]; }

  Model& GetPlayer() {
    for (u32 i = 0; i < kMaxModels; i++) {
      Model& model = *(Model*)((uptr)overworld_models_ + Model::kSize * i);
      if (model.id == kPlayerId) {
        return model;
      }
    }
    return overworld_models_[0];
  }

  INLINE Model& GetModel(u32 idx) {
    return *(Model*)((uptr)overworld_models_ + Model::kSize * idx);
  }

  INLINE void* Get3DModelManager() {
    return *(void**)((uptr)&resource_count_ - 8);
  }

  static constexpr u32 kMaxModels = 32;

  INLINE void* Get3DModelManager() const { return move_3d_model_manager_; }

  INLINE renderer::H3dResource* GetShaderResource() const {
    if (move_3d_model_manager_ == nullptr) return nullptr;
    auto* resource = *(renderer::H3dResource**)(
      (uptr)move_3d_model_manager_ + kCharacterShaderOffset);
    if (resource == nullptr || !resource->IsValid()) return nullptr;
    return resource;
  }

private:
  static constexpr u32 kCharacterShaderOffset = 0x2EA8;

  void* heap_;
  game::DataManager* game_data_manager_;
  void* _0[5];
  u8 _1[0x3680];
  Model* overworld_models_;
  u32 overworld_model_count_;

  void* _2[6]; // collision, etc.
  void* move_3d_model_manager_;
  void* effect_manager_;

public:
  u32 resource_count_;
  u32 resource_count_2_;
  ModelResource resources_[kMaxModels];
};
} // namespace overworld