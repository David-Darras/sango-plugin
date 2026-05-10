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

#include "menu/plugin_menu.h"
#include "overworld/tile.h"

#include "hack/hook_manager.h"

namespace overworld {
#include "overworld/tile.inc"

static struct {
  bool is_enabled;
  bool is_impassable;
  bool is_water;
  bool permits_encounters;
  bool allows_diagonal_movement;
  bool has_reflection_0;
  bool has_shadow;
  bool blocks_trainer_movement;
  bool can_place_decorations;
  bool can_move_decoration_cursor;
  bool has_reflection_1;
  bool has_reflection_2;
  bool has_reflection_3;
  u8 foot_step_id;
  u8 battle_background_id;
  u8 ground_id;
} ctx{};

Tile OnGetTile(void* data, Vec3* pos) {
  Tile tile = HookManager::GetInstance().Get(HookID::kOnGetTile)->CallOriginal<
    Tile>(data, pos);

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
  }

  return tile;
}

void Tile::LoadMenu(menu::PluginMenu& menu, void* args) {
  HookManager::GetInstance().Add(HookID::kOnGetTile, ADDRESS_MAP_GET_TILE,
                                 (uptr)OnGetTile);

  menu.Add("Is Enabled", ctx.is_enabled)
      .AddSeparator()
      .Add("Is Impassable", ctx.is_impassable)
      .Add("Is Water", ctx.is_water)
      .Add("Permits Encounters", ctx.permits_encounters)
      .Add("Allows Diagonal Movement", ctx.allows_diagonal_movement)
      .Add("Has Shadow", ctx.has_shadow)
      .Add("Blocks Trainer Movement", ctx.blocks_trainer_movement)
      .Add("Can Place Decorations", ctx.can_place_decorations)
      .Add("Can Move Decoration Cursor", ctx.can_move_decoration_cursor)
      .AddSeparator()
      .Add("Reflection 0", ctx.has_reflection_0)
      .Add("Reflection 1", ctx.has_reflection_1)
      .Add("Reflection 2", ctx.has_reflection_2)
      .Add("Reflection 3", ctx.has_reflection_3)
      .AddSeparator()
      .Add("Footstep Sound Type", ctx.foot_step_id)
      .WithArray(FOOT_STEPS, SIZE(FOOT_STEPS))
      .Add("Battle Background Type", ctx.battle_background_id)
      //.WithArray(battle::BACKGROUNDS, battle::BACKGROUNDS_COUNT)
      .Add("Ground Type", ctx.ground_id)
      .WithArray(GROUNDS, SIZE(GROUNDS));
}
} // namespace overworld