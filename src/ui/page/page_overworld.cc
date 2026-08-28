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

#include "feature/feature_app.h"
#include "feature/feature_camera.h"
#include "feature/feature_day_care.h"
#include "feature/feature_field_move.h"
#include "feature/feature_map_tile.h"
#include "feature/feature_overworld_model.h"
#include "game/constant/map.h"
#include "ui/main_application.h"
#include "game/overworld/encounter.h"
#include "game/overworld/prop_model_manager.h"
#include "game/overworld/weather_manager.h"
#include "game/renderer/h3d_shader_model.h"
#include "ui/page/page_top.h"

namespace ui {
#include "game/overworld/tile.inc"

void LoadOverworldMapTilePage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  auto& ctx = feature::MapTile::GetInstance();

  app.Add("Is Enabled", ctx.is_enabled)
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
     .Add("Ground Type", ctx.ground_id)
     .WithArray(GROUNDS, SIZE(GROUNDS));
}

void LoadOverworldEncounterPage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  auto& data = overworld::Encounter::GetInstance();

  app.Add("Walk Count", data.walk_count)
     .Add("Encounter Rate", data.encounter_rate)
     .Add("Fishing Chain", data.fishing_chain_count);
}

void LoadOverworldFieldMovePage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  static u32 choice = 0;

  static const c8* MOVES[] = {
      "Cut", "Surf", "Waterfall",
      "Strength", "Rock Smash", "Fly",
      "Flash", "Teleport", "Dig",
      "Sweet Scent", "Dive", "Secret Power"
  };

  app.Add("Field Move", choice);
  app.WithArray(MOVES, SIZE(MOVES));
  app.Add("Execute", [&](void*) { feature::FieldMove::Execute(choice); });
}

void LoadOverworldCameraPage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  static const c8* STATES[] = {"Idle", "Tps", "Rotate", "Top", "Fps", "Free"};
  auto& ctx = feature::Camera::GetInstance();

  app.WithNoBackground()
     .Add("State", ctx.overworld_state)
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

void LoadOverworldModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  auto& ctx = feature::OverworldModel::GetInstance();
  auto& man = overworld::ModelManager::GetInstance();
  auto& rsrc = man.GetResource(ctx.model_idx);
  auto& draw_model = man.GetPlayer().GetDrawModel();

  app.Add("Scale X", draw_model.scale.x)
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
     .WithCallback(feature::OverworldModel::PlayAnimation);
}

void LoadPropModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;
  auto& manager = overworld::PropModelManager::GetInstance();
  if (manager.count == 0) {
    app.Add("There is no prop here...");
    return;
  }

  static const char* PROP_SOUND_EFFECTS[] = {
      "None",
      "Normal Door",
      "Push Door",
      "Double Door",
      "Automatic Door",
      "Pokemon Center",
      "Big Double Door",
      "Temple Door",
      "Metal Door",
      "Elite Four Door"
  };

  static u32 choice = 0;
  ui::LogApplication::Print(u"man=%p", &manager);

  app.Add("Choice", choice)
     .WithBounds(0, manager.count - 1)
     .WithRefresh();

  auto& prop = manager.prop_models[choice];
  if (!prop.is_initialized) return;

  auto* shader = manager.prop_models[choice].shader;

  app.AddSeparator()
     .Add("Is Visible", shader->_0[0xE4])
     .Add("Sound Effect", prop.sound_effect)
     .WithArray(PROP_SOUND_EFFECTS, SIZE(PROP_SOUND_EFFECTS))
     .AddSeparator()
     .Add("Scale X", shader->scale.x)
     .WithFactor(0.2f)
     .Add("Scale Y", shader->scale.y)
     .WithFactor(0.2f)
     .Add("Scale Z", shader->scale.z)
     .WithFactor(0.2f)
     .AddSeparator()
     .Add("Rotation X", shader->rotation.x)
     .WithFactor(M_PI / 12.0f)
     .Add("Rotation Y", shader->rotation.y)
     .WithFactor(M_PI / 12.0f)
     .Add("Rotation Z", shader->rotation.z)
     .WithFactor(M_PI / 12.0f)
     .AddSeparator()
     .Add("Position X", shader->position.x)
     .Add("Position Y", shader->position.y)
     .Add("Position Z", shader->position.z)
     .AddSeparator();
}

void LoadAppPage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  auto& ctx = feature::GameApp::GetInstance();

  const std::pair<const char*, u32> apps[] = {
      // {"Tutor", 7},
      {"PC", 17},
      {"Remind", 9},
      {"Delete", 8}
  };

  for (const auto& app_pair : apps) {
    app.Add(app_pair.first, [&ctx, app_pair](void*) {
      ctx.TriggerApp(app_pair.second);
    });
  }
}

void LoadDayCarePage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  auto& day_care = feature::DayCare::GetInstance();

  app.Add("Instant Egg Hatch", day_care.instant_egg_hatch)
     .Add("Instant Egg Generation", day_care.instant_egg_generation)
     .Add("Instant Max Exp", day_care.instant_max_exp);
}

// static u16 map_id = MAP_INSIDE_OF_TRUCK;
//
// void Teleport(void*) {
//   static const u32 PARAMS[] = {
//       40, map_id, 0, 0, 0, 0, 0, 1, 0, 1
//   };
//   ((void(*)(uptr, const u32*))0x00747B4C)(0, PARAMS);
// }


void LoadOverworldPage(MainApplication& app, void* args) {
  if (app.CheckProcess(ADDRESS_OVERWORLD_VTABLE)) return;

  static const c8* WEATHERS[] = {
      "Sunny", "Rainy", "Thunderstorm",
      "Misty", "Ash", "Sandstorm",
      "Cloudy", "Stormy", "Dry"
  };

  auto& weather_manager = overworld::WeatherManager::GetInstance();
  auto& man = overworld::MapManager::GetInstance();
  app
      // .WithNoBackground()
      // .Add("Teleport", map_id)
      // .WithCallback(Teleport)
      .Add("Weather", weather_manager.GetRequestedWeather())
      .WithArray(WEATHERS, SIZE(WEATHERS))
      .Add("App", LoadAppPage)
      .Add("Camera", LoadOverworldCameraPage)
      .Add("Model Loader (Instable)", LoadModelLoaderPage)
      .Add("Prop", LoadPropModelPage)
      .Add("Player", LoadOverworldModelPage)
      .Add("Encounter", LoadOverworldEncounterPage)
      .Add("Map Tile", LoadOverworldMapTilePage)
      .Add("Field Move", LoadOverworldFieldMovePage)
      .Add("Day Care", LoadDayCarePage)
      .Add("Map Id", man.GetMapId());
}
} // namespace ui