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
#include "overworld/map_manager.h"

namespace feature {
struct FieldMove {
  static void Execute(u32 choice) {
    auto& map_manager = overworld::MapManager::GetInstance();

    struct {
      u16 zone_id;
      u16 team_index;
      overworld::MapManager* map_manager;
    } context = {(u16)map_manager.GetMapId(), 0, &map_manager};

    ((void (*)(void*, u32))ADDRESS_DO_FIELD_MOVE)(&context, choice);
  }
};
}