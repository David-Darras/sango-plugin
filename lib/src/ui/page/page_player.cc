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

#include <utility>

#include "core/patch/app_launcher.h"
#include "overworld/native/model_manager.h"
#include "overworld/patch/camera.h"
#include "overworld/patch/field_move.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/patch/run_animation.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
void LoadOverworldFieldMovePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const std::pair<const c8*, FieldMoveId> kMoves[] = {
      {"Cut", FieldMoveId::kCut},
      {"Rock Smash", FieldMoveId::kRockSmash},
      {"Strength", FieldMoveId::kStrength},
      {"Surf", FieldMoveId::kSurf},
      {"Dive", FieldMoveId::kDive},
      {"Waterfall", FieldMoveId::kWaterfall},
      {"Flash", FieldMoveId::kFlash},
      {"Teleport", FieldMoveId::kTeleport},
      {"Dig", FieldMoveId::kDig},
      {"Sweet Scent", FieldMoveId::kSweetScent},
      {"Secret Power", FieldMoveId::kSecretPower},
  };

  // Fly goes through the town map, not through the field move menu.
  app.Add("Fly", [](void*) { core::AppLauncher::DoFly(); });
  for (const auto& move : kMoves) {
    app.Add(move.first, [move](void*) {
      overworld::FieldMove::Execute(move.second);
    });
  }
}

static void WonderTrade(void*) {
  static u32 input[] = {0, /*index*/0, /*team*/0xFE, /*team_index*/0};
  ((void(*)(uptr, u32*))overworld::address::kTradePokemon)(0, input);

  MainApplication::GetInstance().ForceClose();
}

void LoadAppPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& launcher = core::AppLauncher::GetInstance();

  // The move tutor is left out: it teaches one hardcoded move.
  static const std::pair<const c8*, AppId> kApps[] = {
      {"PC", AppId::kBox},
      {"Move Reminder", AppId::kMoveReminder},
      {"Move Deleter", AppId::kMoveDeleter},
  };
  for (const auto& entry : kApps) {
    app.Add(entry.first, [&launcher, entry](void*) {
      launcher.TriggerApp(entry.second);
    });
  }
}

void LoadOverworldCameraPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const c8* STATES[] = {"Idle", "Tps", "Rotate", "Top", "Fpv", "Free"};
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
     .Add("FPV Height", ctx.fpv_height)
     .Add("FPV Offset", ctx.fpv_offset)
     .AddSeparator()
     .Add("Orbit Radius", ctx.radius)
     .WithFactor(3.0f)
     .Add("Orbit Height", ctx.height)
     .WithFactor(3.0f)
     .Add("Orbit Rotation Speed", ctx.theta_speed)
     .WithFactor(0.01f);
}

#ifdef GAME_XY
// The draw code reads the player's own ObjCodeParam copy, not the area list:
// push the edited copy into the list entry too, then rebuild the map.
static void ApplyPlayerModel(void*) {
  auto& cheats = overworld::PlayerCheats::GetInstance();
  auto& man = overworld::ModelManager::GetInstance();
  auto& player_param = man.GetPlayer().GetObjCodeParam();
  auto& resource = man.GetResource(cheats.model_idx);
  resource.model_id = player_param.model_id;
  resource.dress_up_flag = player_param.dress_up_flag;
  RefreshMap(nullptr);
}
#endif

void LoadPlayerModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& cheats = overworld::PlayerCheats::GetInstance();
  auto& man = overworld::ModelManager::GetInstance();
  auto& resource = man.GetResource(cheats.model_idx);
  auto& draw_model = man.GetPlayer().GetDrawModel();

  app.WithNoBackground()
#ifdef GAME_XY
     .Add("Model", man.GetPlayer().GetObjCodeParam().model_id)
     .WithCallback(ApplyPlayerModel)
     .Add("Use Outfit", man.GetPlayer().GetObjCodeParam().dress_up_flag)
     .WithBounds(0, 1)
     .WithCallback(ApplyPlayerModel)
#else
     .Add("Model", resource.model_id)
     .WithCallback(RefreshMap)
#endif
     .Add("Animation", cheats.model_animation)
     .WithCallback(overworld::PlayerCheats::PlayAnimation)
     .AddSeparator()
     .Add("Scale X", draw_model.scale.x)
     .WithFactor(0.2f)
     .Add("Scale Y", draw_model.scale.y)
     .WithFactor(0.2f)
     .Add("Scale Z", draw_model.scale.z)
     .WithFactor(0.2f)
     .AddSeparator()
     .Add("Model Index", cheats.model_idx)
     .WithBounds(0, overworld::ModelManager::kMaxModels - 1)
     .WithRefresh();
}

void LoadPlayerPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& cheats = overworld::PlayerCheats::GetInstance();
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();

  app.Add("Field Moves", LoadOverworldFieldMovePage)
     .Add("Apps", LoadAppPage)
     .Add("Wonder Trade", WonderTrade)
     .Add("Camera", LoadOverworldCameraPage)
     .Add("Model", LoadPlayerModelPage)
     .AddSeparator()
     .Add("Noclip", CheatCodeId::kNoclip)
     .Add("Noclip Speed X", cheats.speed.x)
     .Add("Noclip Speed Y", cheats.speed.y)
     .Add("Noclip Speed Z", cheats.speed.z)
     .AddSeparator()
     .Add("Swarm Mod", CheatCodeId::kSwarmMod)
     .Add("Swarm Radius", cheats.radius)
     .Add("Swarm Rotation Speed", cheats.theta_speed)
     .AddSeparator()
     .Add("Run Animation", overworld::RunAnimation::GetInstance().enabled)
     .Add("Tile X", player.map_pos.coords.x)
     .Add("Tile Y", player.map_pos.coords.y)
     .Add("Tile Z", player.map_pos.coords.z);
}
} // namespace ui
