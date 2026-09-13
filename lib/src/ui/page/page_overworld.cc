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

#include <cmath>

#include "ui/log_application.h"
#include "core/patch/app_launcher.h"
#include "overworld/patch/camera.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/field_move.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/map_graft.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/map_tile.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/constant/map.h"
#include "ui/main_application.h"
#include "overworld/native/encounter_state.h"
#include "overworld/native/prop_model_manager.h"
#include "overworld/native/weather_manager.h"
#include "overworld/patch/weather_override.h"
#include "renderer/native/h3d_shader_model.h"
#include "ui/page/pages.h"
#include "ui/tile_editor_application.h"

namespace ui {
#include "overworld/data/tile.inc"

static void RefreshMap(void*) {
  auto& main_app = MainApplication::GetInstance();
  if (main_app.CheckProcess(overworld::address::kVtable)) return;

  const overworld::Position& pos =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  const bool same_background_music = true;
  const bool show_map_name = false;
  overworld::MapManager::ChangeMap(overworld::MapManager::GetInstance().GetMap(),
                                   pos, overworld::Facing::kUp,
                                   same_background_music, show_map_name);

  main_app.ForceClose();
}

void LoadOverworldMapTilePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& ctx = overworld::MapTile::GetInstance();

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
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& data = overworld::EncounterState::GetInstance();

  app.Add("Walk Count", data.walk_count)
     .Add("Encounter Rate", data.encounter_rate)
     .Add("Fishing Chain", data.fishing_chain_count);
}

void LoadOverworldFieldMovePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  app.Add("Cut", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kCut);
  });
  app.Add("Rock Smash", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kRockSmash);
  });
  app.Add("Strength", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kStrength);
  });
  app.Add("Fly", [](void*) {
    core::AppLauncher::DoFly();
  });
  app.Add("Surf", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kSurf);
  });
  app.Add("Dive", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kDive);
  });
  app.Add("Waterfall", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kWaterfall);
  });
  app.Add("Sweet Scent", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kSweetScent);
  });
  app.Add("Flash", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kFlash);
  });
  app.Add("Teleport", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kTeleport);
  });
  app.Add("Dig", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kDig);
  });
  app.Add("Secret Power", [](void*) {
    overworld::FieldMove::Execute(FieldMoveId::kSecretPower);
  });
}

void LoadOverworldCameraPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const c8* STATES[] = {"Idle", "Tps", "Rotate", "Top", "Fps", "Free"};
  auto& ctx = overworld::Camera::GetInstance();

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
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& ctx = overworld::PlayerCheats::GetInstance();
  auto& man = overworld::ModelManager::GetInstance();
  auto& rsrc = man.GetResource(ctx.model_idx);
  auto& draw_model = man.GetPlayer().GetDrawModel();

  app.WithNoBackground()
     .Add("Model", rsrc.model_id)
     .WithCallback(RefreshMap)
     .Add("Animation", ctx.model_animation)
     .WithCallback(overworld::PlayerCheats::PlayAnimation)
     .AddSeparator()
     .Add("Scale X", draw_model.scale.x)
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
     .WithRefresh();
}

void LoadPropModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;
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
  LogApplication::Print(u"man=%p", &manager);

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
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& ctx = core::AppLauncher::GetInstance();

  const std::pair<const char*, AppId> apps[] = {
      // {"Tutor", AppId::kMoveTutor},
      {"PC", AppId::kBox},
      {"Remind", AppId::kMoveReminder},
      {"Delete", AppId::kMoveDeleter}
  };

  for (const auto& app_pair : apps) {
    app.Add(app_pair.first, [&ctx, app_pair](void*) {
      ctx.TriggerApp(app_pair.second);
    });
  }
}

void LoadDayCarePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& day_care = overworld::DayCare::GetInstance();

  app.Add("Instant Egg Hatch", day_care.instant_egg_hatch)
     .WithCallback(overworld::DayCare::ApplyEggHatch)
     .Add("Instant Max Exp", day_care.instant_max_exp)
     .WithCallback(overworld::DayCare::ApplyMaxExp);
}

static void WonderTrade(void*) {
  static u32 input[] = {0, /*index*/0, /*team*/0xFE, /*team_index*/0};
  ((void(*)(uptr, u32*))overworld::address::kTradePokemon)(0, input);

  MainApplication::GetInstance().ForceClose();
}

void LoadWorldLayoutPage(MainApplication& app, void* args) {
  using overworld::WorldLayout;
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const c8* SIDES[] = {"Up", "Down", "Left", "Right"};
  static const c8* GRAFT_NAMES[overworld::MapGraft::kMaxRequests] = {
      "Graft 0", "Graft 1", "Graft 2", "Graft 3",
      "Graft 4", "Graft 5", "Graft 6", "Graft 7"
  };
  static const c8* LINK_NAMES[overworld::MapGraft::kMaxLinks] = {
      "Link 0", "Link 1", "Link 2", "Link 3",
      "Link 4", "Link 5", "Link 6", "Link 7"
  };

  auto& layout = WorldLayout::GetInstance();
  auto& graft = overworld::MapGraft::GetInstance();

  app.Add("Layout Id", layout.id)
     .Add("Graft Enabled", graft.is_enabled)
     .Add("Graft Logging", graft.is_logging_enabled)
     .Add("Character Logging",
          overworld::MapCharacter::GetInstance().is_logging_enabled)
     .Add("Apply Grafts (Reload Map)", RefreshMap);

  app.AddSeparator()
     .Add("Tile Editor", [](void*) { TileEditorApplication::Open(); })
     .Add("Tile Edits Enabled", overworld::TileEditor::GetInstance().is_enabled)
     .Add("Save Tile Edits", [](void*) { overworld::TileEditor::Save(); })
     .Add("Clear Tile Edits", [](void*) { overworld::TileEditor::ClearEdits(); });

  for (u32 i = 0; i < overworld::MapGraft::GetCount(); i++) {
    auto& request = overworld::MapGraft::GetRequest(i);
    auto& result = overworld::MapGraft::GetResult(i);
    app.AddSeparator()
       .Add(GRAFT_NAMES[i], result.is_applied)
       .Add("Anchor Map", request.anchor)
       .Add("Side", request.side)
       .WithArray(SIDES, SIZE(SIDES))
       .Add("Grafted Map", request.map)
       .Add("Shift (blocks)", request.shift)
       .WithMin(-31)
       .WithMax(31)
       .Add("Offset X (blocks)", result.offset_x)
       .Add("Offset Z (blocks)", result.offset_z);
  }

  for (u32 i = 0; i < overworld::MapGraft::GetLinkCount(); i++) {
    auto& link = overworld::MapGraft::GetLink(i);
    app.AddSeparator()
       .Add(LINK_NAMES[i])
       .Add("Leaving Map", link.from)
       .Add("Through Side", link.side)
       .WithArray(SIDES, SIZE(SIDES))
       .Add("Lands On Map", link.to)
       .Add("Tile X", link.tile_x)
       .WithBounds(0, WorldLayout::kMaxWidth * WorldLayout::kTilesPerBlockSide)
       .Add("Tile Z", link.tile_z)
       .WithBounds(
           0, WorldLayout::kMaxHeight * WorldLayout::kTilesPerBlockSide);
  }
}

void LoadOverworldPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const c8* WEATHERS[] = {
      "Sunny", "Rainy", "Thunderstorm",
      "Misty", "Ash", "Sandstorm",
      "Cloudy", "Stormy", "Dry"
  };

  auto& weather_manager = overworld::WeatherManager::GetInstance();
  auto& man = overworld::MapManager::GetInstance();
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  app.Add("World Layout", LoadWorldLayoutPage);
  app.Add("Wonder Trade", WonderTrade)
     .Add("Refresh", RefreshMap)
     .Add("Weather", weather_manager.GetRequestedWeather())
     .WithArray(WEATHERS, SIZE(WEATHERS))
     .Add("Keep Weather Indoors",
          overworld::WeatherOverride::GetInstance().keep_weather_indoors)
     .Add("Field Move", LoadOverworldFieldMovePage)
     .Add("App", LoadAppPage)
     .Add("Camera", LoadOverworldCameraPage)
     .Add("Model Loader (Unstable)", LoadModelLoaderPage)
     .Add("Prop", LoadPropModelPage)
     .Add("Player", LoadOverworldModelPage)
     .Add("Encounter", LoadOverworldEncounterPage)
     .Add("Map Tile", LoadOverworldMapTilePage)
     .Add("Day Care", LoadDayCarePage)
     .Add("Map Id", man.GetMapId())
     .Add("Player Tile X", player.map_pos.coords.x)
     .Add("Player Tile Y", player.map_pos.coords.y)
     .Add("Player Tile Z", player.map_pos.coords.z);
}
} // namespace ui