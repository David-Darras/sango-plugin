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
#include "core/utils.h"
#include "overworld/patch/camera.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "ui/application_manager.h"
#include "ui/theme.h"

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

  // The same bottom button as the "Back" of the menu.
  app.back_button_.Initialize(4, 206, 312, 30);
  app.back_button_.Reset();
  ApplicationManager::GetInstance().Push(app);
}

void FreeCameraApplication::Update(sys::Controller& controller) {
  const bool in_context = is_battle_
                              ? core::ProcessManager::IsBattleActive()
                              : core::ProcessManager::IsOverworldActive();
  back_button_.Update();
  if (!in_context || controller.IsKeyPressed(Key::kStart) ||
      back_button_.IsReleased()) {
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

namespace {
// One line of the table of the controls: the buttons on the left, what they do
// on the right. A rectangle like the touch buttons of the menu.
void DrawControl(s32 y, const c16* keys, const c16* action) {
  const Theme& theme = Theme::GetInstance();
  Color fill = theme.unselected_text_color;
  fill.a = 0.12f;
  sys::Graphics::DrawRect(4, y, 312, 22, fill);
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  DrawMenuText(12, y + 5, keys, theme.selected_text_color);
  DrawMenuText(110, y + 5, action, theme.unselected_text_color);
}
} // namespace

void FreeCameraApplication::DrawBottom(sys::Graphics& graphics) {
  const Theme& theme = Theme::GetInstance();
  // The look of the menu: the color of the theme over the game.
  sys::Graphics::FillScreen(theme.background_color);

  // The title is quiet, the name of the page is clear, then its description.
  Color dim = theme.unselected_text_color;
  dim.a *= 0.55f;
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  DrawMenuText(6, 3, u"Menu > Free Camera", dim);
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  DrawMenuText(6, 15, u"Free Camera", theme.selected_text_color);
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  DrawMenuText(6, 33, u"Moves the camera with the buttons. Hold a key to "
                      u"go faster.",
               theme.unselected_text_color);

  DrawControl(58, u"D-Pad", u"Move / strafe");
  DrawControl(84, u"{L} / {R}", u"Down / up");
  DrawControl(110, u"{Y} / {A}", u"Turn left / right");
  DrawControl(136, u"{X} / {B}", u"Look up / down");

  // The state of the camera, for the developers (like the process line of
  // the menu).
  auto& camera = overworld::Camera::GetInstance();
  c16 line[96];
  core::Utils::Format(line, u"X %.0f  Y %.0f  Z %.0f   Yaw %.2f  Pitch %.2f",
                      camera.pos.x, camera.pos.y, camera.pos.z, camera.rot.y,
                      camera.rot.x);
  sys::Graphics::SetTextScale(0.4f, 0.4f);
  dim.a = theme.unselected_text_color.a * 0.5f;
  sys::Graphics::DrawText(6, 193, line, dim);

  // The button of the bottom: the same as Back in the menu.
  const bool is_down = back_button_.IsDown();
  Color fill = is_down ? theme.selected_text_color
                       : theme.unselected_text_color;
  fill.a = is_down ? 0.7f : 0.12f;
  sys::Graphics::DrawRect(back_button_.GetX(), back_button_.GetY(),
                          back_button_.GetWidth(), back_button_.GetHeight(),
                          fill);
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  const c16* label = u"Back to the menu   START";
  DrawMenuText(back_button_.GetX() +
                   (back_button_.GetWidth() -
                    sys::Graphics::GetTextWidth(label)) / 2,
               back_button_.GetY() + 9, label, theme.unselected_text_color);
}

} // namespace ui
