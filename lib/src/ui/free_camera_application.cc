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
 * @file free_camera_application.cc
 * @brief The application of the free camera.
 *
 * The declarations are in ui/free_camera_application.h.
 */

#include "ui/free_camera_application.h"

#include <cmath>

#include "core/native/process_manager.h"
#include "overworld/patch/camera.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "ui/application_manager.h"

namespace ui {

static constexpr f32 kMinSpeed = 1.0f;
static constexpr f32 kMaxSpeed = 20.0f;
static constexpr u32 kRampFrames = 120;
static constexpr f32 kMinTurnSpeed = 0.01f;
static constexpr f32 kMaxTurnSpeed = 0.08f;
static constexpr f32 kMaxPitch = 1.5f;

void FreeCameraApplication::Open() {
  const bool is_battle = core::ProcessManager::IsBattleActive();
  if (!is_battle && !core::ProcessManager::IsOverworldActive()) return;

  auto& app = GetInstance();
  app.is_battle_ = is_battle;

  auto& camera = overworld::Camera::GetInstance();
  auto& state = is_battle ? camera.battle_state : camera.overworld_state;
  auto& old_state =
      is_battle ? camera.battle_old_state : camera.overworld_old_state;
  if (state != overworld::CameraState::kFree) {
    state = overworld::CameraState::kFree;
    old_state = overworld::CameraState::kIdle;
  }

  ApplicationManager::GetInstance().Push(app);
}

void FreeCameraApplication::Update(sys::Controller& controller) {
  const bool in_context = is_battle_
                              ? core::ProcessManager::IsBattleActive()
                              : core::ProcessManager::IsOverworldActive();
  if (!in_context || controller.IsKeyPressed(Key::kStart)) {
    ApplicationManager::GetInstance().Pop();
    return;
  }

  auto& camera = overworld::Camera::GetInstance();
  const overworld::CameraState state =
      is_battle_ ? camera.battle_state : camera.overworld_state;
  const overworld::CameraState old_state =
      is_battle_ ? camera.battle_old_state : camera.overworld_old_state;
  if (state != old_state) return;

  f32& yaw = camera.rot.y;
  f32& pitch = camera.rot.x;
  const f32 turn = (controller.IsKeyDown(Key::kA) ? 1.0f : 0.0f) -
                   (controller.IsKeyDown(Key::kY) ? 1.0f : 0.0f);
  const f32 look = (controller.IsKeyDown(Key::kX) ? 1.0f : 0.0f) -
                   (controller.IsKeyDown(Key::kB) ? 1.0f : 0.0f);

  if (turn != 0.0f || look != 0.0f) {
    if (turn_held_frames_ < kRampFrames) turn_held_frames_++;
  } else {
    turn_held_frames_ = 0;
  }
  const f32 turn_speed = kMinTurnSpeed + (kMaxTurnSpeed - kMinTurnSpeed) *
                                             turn_held_frames_ / kRampFrames;

  yaw += turn * turn_speed;
  pitch += look * turn_speed;
  if (pitch > kMaxPitch) pitch = kMaxPitch;
  if (pitch < -kMaxPitch) pitch = -kMaxPitch;

  const f32 forward = (controller.IsKeyDown(Key::kUp) ? 1.0f : 0.0f) -
                      (controller.IsKeyDown(Key::kDown) ? 1.0f : 0.0f);
  const f32 strafe = (controller.IsKeyDown(Key::kRight) ? 1.0f : 0.0f) -
                     (controller.IsKeyDown(Key::kLeft) ? 1.0f : 0.0f);
  const f32 lift = (controller.IsKeyDown(Key::kR) ? 1.0f : 0.0f) -
                   (controller.IsKeyDown(Key::kL) ? 1.0f : 0.0f);

  if (forward != 0.0f || strafe != 0.0f || lift != 0.0f) {
    if (held_frames_ < kRampFrames) held_frames_++;
  } else {
    held_frames_ = 0;
  }
  const f32 speed =
      kMinSpeed + (kMaxSpeed - kMinSpeed) * held_frames_ / kRampFrames;

  camera.pos.x += speed * (forward * std::cos(yaw) * std::cos(pitch) -
                           strafe * std::sin(yaw));
  camera.pos.y += speed * (forward * std::sin(pitch) + lift);
  camera.pos.z += speed * (forward * std::sin(yaw) * std::cos(pitch) +
                           strafe * std::cos(yaw));
}

void FreeCameraApplication::DrawBottom(sys::Graphics& graphics) {
  sys::Graphics::DrawRect(0, 0, 320, 240, Color(0.0f, 0.0f, 0.0f, 1.0f));
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  const Color text(1.0f, 1.0f, 1.0f, 1.0f);
  const Color muted(0.7f, 0.7f, 0.7f, 1.0f);

  sys::Graphics::DrawText(5, 5, u"Free camera", text);
  sys::Graphics::DrawText(5, 30, u"D-Pad   move / strafe", muted);
  sys::Graphics::DrawText(5, 46, u"L / R   down / up", muted);
  sys::Graphics::DrawText(5, 62, u"Y / A   turn left / right", muted);
  sys::Graphics::DrawText(5, 78, u"X / B   look up / down", muted);
  sys::Graphics::DrawText(5, 94, u"Start   back to the menu", muted);
  sys::Graphics::DrawText(5, 118, u"Hold a key to speed up", muted);
}

} // namespace ui
