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

#include "overworld/patch/field_move.h"
#include "overworld/native/map_manager.h"
#include "ui/main_application.h"

namespace overworld {

void FieldMove::Initialize() {
  ARM_NOP(address::kFlyCheck); // Force fly
}

void FieldMove::Execute(FieldMoveId move) {
  auto& main_app = ui::MainApplication::GetInstance();
  if (main_app.CheckProcess(address::kVtable)) return;

  auto& map_manager = MapManager::GetInstance();

  struct {
    u16 zone_id;
    u16 team_index;
    MapManager* map_manager;
  } context = {8, 0, &map_manager};

  ((void (*)(void*, FieldMoveId))core::address::kDoFieldMove)(&context, move);

  ui::MainApplication::GetInstance().ForceClose();
}

} // namespace overworld
