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
#include "game/overworld/map_manager.h"
#include "game/overworld/model_manager.h"
#include "ui/log_application.h"
#include "ui/main_application.h"

namespace feature {
struct FieldMove {
  MAKE_SINGLETON(FieldMove)
  STATIC_INLINE void Initialize() {
    ARM_NOP(ADDRESS_OVERWORLD_FLY_CHECK); // Force fly
  }

  static void Execute(u32 choice) {
    auto& main_app = ui::MainApplication::GetInstance();
    if (main_app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

    auto& map_manager = overworld::MapManager::GetInstance();

    // EventAmaikaori : 007FA3D0
    // EventSorawotobu : 005DF038

    struct {
      u16 zone_id;
      u16 team_index;
      overworld::MapManager* map_manager;
    } context = {8, 0, &map_manager};

    ((void (*)(void*, u32))ADDRESS_DO_FIELD_MOVE)(&context, choice);

    ui::MainApplication::GetInstance().ForceClose();
  }
};
}