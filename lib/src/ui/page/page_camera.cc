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

/**
 * @file page_camera.cc
 * @brief The menu page of the camera of the overworld, and the entries that
 * the camera page of the battle shares (see camera_sections.h).
 */

#include "overworld/patch/camera.h"
#include "ui/free_camera_application.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/page/camera_sections.h"
#include "overworld/address.h"

namespace ui {
const c8* kCameraStateNames[6] = {"Idle", "Third Person", "Rotate",
                                  "Top", "First Person", "Free"};

void AddCameraState(MainApplication& app, overworld::CameraState& state) {
  app.Add("Free Camera", [](void*) { FreeCameraApplication::Open(); })
     .WithDescription("Moves the camera with the buttons. The bottom screen "
         "shows the controls.")
     .Add("State", state)
     .WithArray(kCameraStateNames, SIZE(kCameraStateNames))
     .WithRefresh();
}

void AddCameraStateSection(MainApplication& app, overworld::CameraState state) {
  auto& ctx = overworld::Camera::GetInstance();
  switch (state) {
    case overworld::CameraState::kFree:
      app.AddSection("Free Position")
         .Add("Free Pos X (Left/Right)", ctx.pos.x)
         .WithFactor(5.0f)
         .Add("Free Pos Y (Up/Down)", ctx.pos.y)
         .WithFactor(5.0f)
         .Add("Free Pos Z (Forward/Back)", ctx.pos.z)
         .WithFactor(5.0f)
         .Add("Free Yaw (Turn)", ctx.rot.y)
         .WithFactor(0.05f)
         .Add("Free Pitch (Look)", ctx.rot.x)
         .WithFactor(0.05f);
      break;
    case overworld::CameraState::kTps:
      app.AddSection("Third Person")
         .Add("Distance", ctx.tps_dist)
         .Add("Height", ctx.tps_height)
         .Add("Shoulder Offset", ctx.tps_offset);
      break;
    case overworld::CameraState::kFpv:
      app.AddSection("First Person")
         .Add("Eye Height", ctx.fpv_height)
         .Add("Eye Offset", ctx.fpv_offset);
      break;
    case overworld::CameraState::kRotate:
      app.AddSection("Orbit")
         .Add("Orbit Radius", ctx.radius)
         .WithFactor(3.0f)
         .Add("Orbit Height", ctx.height)
         .WithFactor(3.0f)
         .Add("Orbit Rotation Speed", ctx.theta_speed)
         .WithFactor(0.01f);
      break;
    default:
      break;
  }
}

void LoadOverworldCameraPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& ctx = overworld::Camera::GetInstance();
  AddCameraState(app, ctx.overworld_state);
  // The section of the mode that the camera uses now.
  AddCameraStateSection(app, ctx.overworld_state);
}
} // namespace ui
