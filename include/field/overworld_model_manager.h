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

#ifndef SANGO_PLUGIN_FIELD_OVERWORLD_MODEL_MANAGER_H
#define SANGO_PLUGIN_FIELD_OVERWORLD_MODEL_MANAGER_H

#include "common.h"
#include "system/game_data_manager.h"

namespace field {

struct OverworldModel {};

struct OverworldModelResource {
  u32 _0[5];
  u16 model_id;
  u16 padding;
};

class OverworldModelManager {
 public:
  static OverworldModelManager& GetInstance() {
    return GameDataManager::GetInstance().GetOverworldModelManager();
  }

  static void LoadMenu(menu::PluginMenu& menu, void* args);

  OverworldModelResource& GetResource(u32 idx) { return resources_[idx]; }

 private:
  void* heap_;
  GameDataManager* game_data_manager_;
  void* _0[5];
  u8 _1[0x3680];
  OverworldModel* overworld_models_;
  u32 overworld_model_count_;

  u32 _2[8];

  u32 resource_count_;
  u32 resource_count_2_;
  OverworldModelResource resources_[33];
};

}  // namespace field

#endif  // SANGO_PLUGIN_FIELD_OVERWORLD_MODEL_MANAGER_H
