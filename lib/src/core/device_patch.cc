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
 * @file device_patch.cc
 * @brief Hides the buttons from the game when the menu of the plugin is open.
 *
 * The declarations are in core/patch/device_patch.h.
 */

#include "core/patch/device_patch.h"
#include "core/hook.h"
#include "system/native/device.h"
#include "system/address.h"

namespace core {
namespace {

// Returns true when the game must not see the buttons of this channel.
bool IsHidden(u8 channel) {
  return DevicePatch::GetInstance().use_redirection &&
         channel != sys::Device::kCustomChannel;
}

} // namespace

HOOK(bool, IsKeyPressed, (void* device, u32 key, u8 channel),
     sys::address::kControllerIsKeyPressed) {
  if (IsHidden(channel)) return false;
  return original(device, key, channel);
}

HOOK(bool, IsKeyReleased, (void* device, u32 key, u8 channel),
     sys::address::kControllerIsKeyReleased) {
  if (IsHidden(channel)) return false;
  return original(device, key, channel);
}

HOOK(bool, IsKeyDown, (void* device, u32 key, u8 channel),
     sys::address::kControllerIsKeyDown) {
  if (IsHidden(channel)) return false;
  return original(device, key, channel);
}

HOOK(bool, IsKeyRepeated, (void* device, u32 key, u8 channel),
     sys::address::kControllerIsKeyRepeated) {
  if (IsHidden(channel)) return false;
  return original(device, key, channel);
}

HOOK(bool, IsDPadDown, (void* device, u32 key, u8 channel),
     sys::address::kDpadIsDown2) {
  if (IsHidden(channel)) return false;
  return original(device, key, channel);
}

HOOK(bool, IsTouchDown, (void* touch, u8 channel),
     sys::address::kTouchscreenIsDown) {
  if (IsHidden(channel)) return false;
  return original(touch, channel);
}

HOOK(bool, IsTouchReleased, (void* touch, u8 channel),
     sys::address::kTouchscreenIsReleased) {
  if (IsHidden(channel)) return false;
  return original(touch, channel);
}

HOOK(u32, GetRepeatedKey, (uptr button, u8 channel),
     sys::address::kControllerGetRepeatedKey) {
  if (IsHidden(channel)) return 0;
  return original(button, channel);
}

#ifdef GAME_ORAS
// The game also reads the masks of the buttons (a menu that tests many buttons
// at one time), the directions (the D-pad and the Circle Pad, as a vector) and
// the other states of the touch screen. They have no channel: they are all
// hidden while the menu is open.
HOOK(u32, GetNewButtonsMask, (void* device, u8 channel),
     sys::address::kControllerGetNewButtons) {
  if (IsHidden(channel)) return 0;
  return original(device, channel);
}

HOOK(u32, GetReleasedButtonsMask, (void* device, u8 channel),
     sys::address::kControllerGetReleasedButtons) {
  if (IsHidden(channel)) return 0;
  return original(device, channel);
}

HOOK(bool, IsDPadHold, (void* device, u32 mask), sys::address::kDpadIsDown) {
  if (DevicePatch::GetInstance().use_redirection) return false;
  return original(device, mask);
}

HOOK(bool, IsDPadRepeated, (void* device, u32 mask),
     sys::address::kDpadIsRepeated) {
  if (DevicePatch::GetInstance().use_redirection) return false;
  return original(device, mask);
}

HOOK(bool, IsDPadPressed, (void* device, u32 mask),
     sys::address::kDpadIsPressed) {
  if (DevicePatch::GetInstance().use_redirection) return false;
  return original(device, mask);
}

HOOK(bool, IsDPadReleased, (void* device, u32 mask),
     sys::address::kDpadIsReleased2) {
  if (DevicePatch::GetInstance().use_redirection) return false;
  return original(device, mask);
}

HOOK(s32, IsTouchPressed, (void* touch), sys::address::kTouchscreenIsPressed) {
  if (DevicePatch::GetInstance().use_redirection) return 0;
  return original(touch);
}

HOOK(bool, IsTouchRepeated, (void* touch), sys::address::kTouchscreenIsRepeated) {
  if (DevicePatch::GetInstance().use_redirection) return false;
  return original(touch);
}

// The Circle Pad (the position of the stick, from -1 to 1): the menu does not
// use it, and the game moves its cursors with it.
HOOK(f32, GetStickX, (void* stick), sys::address::kCirclePadReadX) {
  if (DevicePatch::GetInstance().use_redirection) return 0.0f;
  return original(stick);
}

HOOK(f32, GetStickY, (void* stick), sys::address::kCirclePadReadY) {
  if (DevicePatch::GetInstance().use_redirection) return 0.0f;
  return original(stick);
}
#endif

} // namespace core
