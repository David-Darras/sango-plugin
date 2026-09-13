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

#include "overworld/patch/map_tile.h"

#include "core/hook_manager.h"
#include "overworld/patch/tile_editor.h"

namespace overworld {

void MapTile::Initialize() {
  core::HookManager::Initialize(HookId::kGetMapTile, address::kGetMapTile,
                          (uptr)GetMapTileHook);
}

Tile MapTile::GetMapTileHook(void* data, Vec3* pos) {
  auto& ctx = GetInstance();
  TileEditor::OnAttrQuery(data, pos);

  Tile tile = core::HookManager::GetInstance().Get(HookId::kGetMapTile)->
      CallOriginal<Tile>(data, pos);

  if (ctx.is_enabled) {
    tile.is_impassable = ctx.is_impassable;
    tile.is_water = ctx.is_water;
    tile.permits_encounters = ctx.permits_encounters;
    tile.allows_diagonal_movement = ctx.allows_diagonal_movement;
    tile.has_reflection_0 = ctx.has_reflection_0;
    tile.has_shadow = ctx.has_shadow;
    tile.blocks_trainer_movement = ctx.blocks_trainer_movement;
    tile.can_place_decorations = ctx.can_place_decorations;
    tile.can_move_decoration_cursor = ctx.can_move_decoration_cursor;
    tile.has_reflection_1 = ctx.has_reflection_1;
    tile.has_reflection_2 = ctx.has_reflection_2;
    tile.has_reflection_3 = ctx.has_reflection_3;
    tile.foot_step_id = ctx.foot_step_id;
    tile.battle_background_id = ctx.battle_background_id;
    tile.ground_id = ctx.ground_id;
  } else {
    ctx.is_impassable = tile.is_impassable;
    ctx.is_water = tile.is_water;
    ctx.permits_encounters = tile.permits_encounters;
    ctx.allows_diagonal_movement = tile.allows_diagonal_movement;
    ctx.has_reflection_0 = tile.has_reflection_0;
    ctx.has_shadow = tile.has_shadow;
    ctx.blocks_trainer_movement = tile.blocks_trainer_movement;
    ctx.can_place_decorations = tile.can_place_decorations;
    ctx.can_move_decoration_cursor = tile.can_move_decoration_cursor;
    ctx.has_reflection_1 = tile.has_reflection_1;
    ctx.has_reflection_2 = tile.has_reflection_2;
    ctx.has_reflection_3 = tile.has_reflection_3;
    ctx.foot_step_id = tile.foot_step_id;
    ctx.battle_background_id = tile.battle_background_id;
    ctx.ground_id = tile.ground_id;
  }

  return tile;
}

} // namespace overworld
