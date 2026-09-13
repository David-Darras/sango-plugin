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

#include "core/patch/device_patch.h"
#include "core/hook_manager.h"
#include "system/native/device.h"

namespace core {

#define ADD_HOOK(ID, Address) HookManager::Initialize(HookId::ID, Address, (uptr)Hook##ID)

void DevicePatch::Initialize() {
  ADD_HOOK(kIsKeyPressed, sys::address::kControllerIsKeyPressed);
  ADD_HOOK(kIsKeyReleased, sys::address::kControllerIsKeyReleased);
  ADD_HOOK(kIsKeyDown, sys::address::kControllerIsKeyDown);
  ADD_HOOK(kIsKeyRepeated, sys::address::kControllerIsKeyRepeated);
  ADD_HOOK(kIsDPadDown, sys::address::kDpadIsDown2);
  ADD_HOOK(kIsTouchDown, sys::address::kTouchscreenIsDown);
  ADD_HOOK(kIsTouchReleased, sys::address::kTouchscreenIsReleased);

  HookManager::Initialize(HookId::kGetRepeatedKey,
                          sys::address::kControllerGetRepeatedKey,
                          (uptr)GetRepeatKeyHook);
}

#undef ADD_HOOK

u32 DevicePatch::GetRepeatKeyHook(uptr button, u8 channel) {
  if (GetInstance().use_redirection &&
      channel != sys::Device::kCustomChannel)
    return 0;

  return HookManager::Call<u32>(HookId::kGetRepeatedKey, button, channel);
}

#define DEFINE_INPUT_HOOK(FuncName)                         \
bool DevicePatch::Hook##FuncName(void *pDevice, u32 key, u8 channel) { \
  if (DevicePatch::GetInstance().use_redirection &&       \
      channel != sys::Device::kCustomChannel)                  \
    return false;                                         \
  return HookManager::Call<bool>(HookId::FuncName, pDevice, key, channel);\
}

DEFINE_INPUT_HOOK(kIsKeyPressed)
DEFINE_INPUT_HOOK(kIsKeyReleased)
DEFINE_INPUT_HOOK(kIsKeyDown)
DEFINE_INPUT_HOOK(kIsKeyRepeated)
DEFINE_INPUT_HOOK(kIsDPadDown)
DEFINE_INPUT_HOOK(kIsDPadRepeated)

#undef DEFINE_INPUT_HOOK

#define DEFINE_TOUCH_HOOK(FuncName)                   \
bool DevicePatch::Hook##FuncName(void *pTouch, u8 channel) {     \
  if (DevicePatch::GetInstance().use_redirection && \
      channel != sys::Device::kCustomChannel)            \
    return false;                                   \
  return HookManager::Call<bool>(HookId::FuncName, pTouch, channel);        \
}

DEFINE_TOUCH_HOOK(kIsTouchDown)
DEFINE_TOUCH_HOOK(kIsTouchReleased)

#undef DEFINE_TOUCH_HOOK

} // namespace core
