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

#include "feature/feature_camera.h"
#include "feature/feature_field_move.h"
#include "feature/feature_map_tile.h"
#include "feature/feature_overworld_model.h"
#include "menu/plugin_menu.h"
#include "overworld/encounter.h"

namespace ui {
#include "overworld/tile.inc"

void LoadOverworldMapTileMenu(menu::PluginMenu& menu, void* args) {
  auto& ctx = feature::MapTileHookContext::GetInstance();

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

void LoadOverworldEncounterMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_FIELD_MAP)) return;

  auto& data = overworld::Encounter::GetInstance();

  menu.Add("Walk Count", data.walk_count)
      .Add("Encounter Rate", data.encounter_rate)
      .Add("Fishing Chain", data.fishing_chain_count);
}

void LoadOverworldFieldMoveMenu(menu::PluginMenu& menu, void* args) {
  static u32 choice = 0;

  if (menu.CheckProcess(PROCESS_NAME_FIELD_MAP)) return;

  static const c8* MOVES[] = {
      "Cut", "Surf", "Waterfall",
      "Strength", "Rock Smash", "Fly",
      "Flash", "Teleport", "Dig",
      "Sweet Scent", "Dive", "Secret Power"
  };

  menu.Add("Field Move", choice);
  menu.WithArray(MOVES, SIZE(MOVES));
  menu.Add("Execute", [&](void*) { feature::FieldMove::Execute(choice); });
}

void LoadOverworldCameraMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_FIELD_MAP)) return;

  static const c8* STATES[] = {"Idle", "Free", "Rotate", "Fpv", "Tps"};
  bool& skybox = *(bool*)((uptr)&overworld::Renderer::GetInstance() + 0xB74);
  auto& ctx = feature::CameraHookContext::GetInstance();

  menu.WithNoBackground()
      .Add("Skybox", skybox)
      .AddSeparator()
      .Add("State", ctx.state)
      .WithArray(STATES, SIZE(STATES))
      .AddSeparator()
      .Add("Free Pos X (Left/Right)", ctx.pos.x)
      .WithFactor(5.0f)
      .Add("Free Pos Y (Up/Down)", ctx.pos.y)
      .WithFactor(5.0f)
      .Add("Free Pos Z (Forward/Back)", ctx.pos.z)
      .WithFactor(5.0f)
      .Add("Free Yaw (Turn)", ctx.rot.y)
      .WithFactor(0.05f)
      .Add("Free Pitch (Look)", ctx.rot.x)
      .WithFactor(0.05f)
      .AddSeparator()
      .Add("TPS Distance", ctx.tps_dist)
      .Add("TPS Height", ctx.tps_height)
      .Add("TPS Shoulder Offset", ctx.tps_offset)
      .AddSeparator()
      .Add("Radius", ctx.radius)
      .WithFactor(3.0f)
      .Add("Height", ctx.height)
      .WithFactor(3.0f)
      .Add("Orbit Rot Speed", ctx.theta_speed)
      .WithFactor(0.01f);
}

void LoadOverworldModelMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_FIELD_MAP)) return;

  auto& ctx = feature::OverworldModelCheatCode::GetInstance();
  auto& man = overworld::ModelManager::GetInstance();
  auto& rsrc = man.GetResource(ctx.model_idx);
  auto& draw_model = man.GetPlayer().GetDrawModel();

  menu.Add("Scale X", draw_model.scale.x)
      .WithFactor(0.2f)
      .Add("Scale Y", draw_model.scale.y)
      .WithFactor(0.2f)
      .Add("Scale Z", draw_model.scale.z)
      .WithFactor(0.2f)
      .AddSeparator()
      .Add("Noclip", CheatCodeId::kNoclip)
      .Add("Speed-X", ctx.speed.x)
      .Add("Speed-Y", ctx.speed.y)
      .Add("Speed-Z", ctx.speed.z)
      .AddSeparator()
      .Add("Swarm Mod", CheatCodeId::kSwarmMod)
      .Add("Circle Radius", ctx.radius)
      .Add("Rotation Speed", ctx.theta_speed)
      .AddSeparator()
      .Add("Model Index", ctx.model_idx)
      .WithBounds(0, overworld::ModelManager::kMaxModels - 1)
      .WithRefresh()
      .Add("Model", rsrc.model_id)
      .Add("Animation", ctx.model_animation)
      .WithCallback(feature::OverworldModelCheatCode::PlayAnimation);
}

void LoadOverworldMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_FIELD_MAP)) return;

  auto& man = overworld::MapManager::GetInstance();
  menu.Add("Reload Map", man.GetMapId())
      .Add("Encounter", LoadOverworldEncounterMenu)
      .Add("Map Tile", LoadOverworldMapTileMenu)
      .Add("Field Move", LoadOverworldFieldMoveMenu)
      .Add("Camera", LoadOverworldCameraMenu);
}
} // namespace ui