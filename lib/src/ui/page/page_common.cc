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

#include "ui/page/page_common.h"

#include "core/native/data_manager.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "ui/main_application.h"

namespace ui {
void LoadColorPage(MainApplication& app, void* args) {
  constexpr f32 kFactor = 0.025f;
  Color& color = *(Color*)args;

  app.Add("Red", color.r).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Green", color.g).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Blue", color.b).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Alpha", color.a).WithFactor(kFactor).WithBounds(0, 1);
}

void LoadColor8Page(MainApplication& app, void* args) {
  Color8& color = *(Color8*)args;

  app.Add("Red", color.r)
      .Add("Green", color.g)
      .Add("Blue", color.b)
      .Add("Alpha", color.a);
}

void RefreshMap(void*) {
  auto& main_app = MainApplication::GetInstance();
  if (main_app.CheckProcess(overworld::address::kVtable)) return;

  const overworld::Position& pos =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  const bool same_background_music = true;
  const bool show_map_name = false;
  overworld::Facing facing = core::DataManager::GetInstance().GetPlayerDirection();
  if (facing >= overworld::Facing::kCount) facing = overworld::Facing::kUp;
  overworld::MapManager::ChangeMap(overworld::MapManager::GetInstance().GetMap(),
                                   pos, facing, same_background_music,
                                   show_map_name);

  main_app.ForceClose();
}
} // namespace ui
