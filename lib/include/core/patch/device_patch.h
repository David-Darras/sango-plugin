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

#pragma once

#include "common.h"

namespace core {

struct DevicePatch {
  MAKE_SINGLETON(DevicePatch)
  bool use_redirection = false;

  static void Initialize();

private:
  static u32 GetRepeatKeyHook(uptr button, u8 channel);

  static bool HookkIsKeyPressed(void* pDevice, u32 key, u8 channel);
  static bool HookkIsKeyReleased(void* pDevice, u32 key, u8 channel);
  static bool HookkIsKeyDown(void* pDevice, u32 key, u8 channel);
  static bool HookkIsKeyRepeated(void* pDevice, u32 key, u8 channel);
  static bool HookkIsDPadDown(void* pDevice, u32 key, u8 channel);
  static bool HookkIsDPadRepeated(void* pDevice, u32 key, u8 channel);

  static bool HookkIsTouchDown(void* pTouch, u8 channel);
  static bool HookkIsTouchReleased(void* pTouch, u8 channel);
};

} // namespace core
