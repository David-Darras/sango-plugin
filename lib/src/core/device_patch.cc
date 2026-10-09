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

} // namespace core
