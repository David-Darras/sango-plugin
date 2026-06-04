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
#include "core/core.h"
#include "feature/hook_manager.h"
#include "system/device.h"

namespace feature {
struct DeviceHookContext {
  MAKE_SINGLETON(DeviceHookContext)
  bool use_redirection = false;

  STATIC_INLINE void Initialize() {
#define ADD_HOOK(ID, Address) HookManager::Initialize(HookID::ID, Address, (uptr)Hook##ID)

    ADD_HOOK(kIsKeyPressed, ADDRESS_CONTROLLER_IS_KEY_PRESSED);
    ADD_HOOK(kIsKeyReleased, ADDRESS_CONTROLLER_IS_KEY_RELEASED);
    ADD_HOOK(kIsKeyDown, ADDRESS_CONTROLLER_IS_KEY_DOWN);
    ADD_HOOK(kIsKeyRepeated, ADDRESS_CONTROLLER_IS_KEY_REPEATED);
    ADD_HOOK(kIsDPadDown, ADDRESS_DPAD_IS_DOWN_2);
    ADD_HOOK(kIsTouchDown, ADDRESS_TOUCHSCREEN_IS_DOWN);
    ADD_HOOK(kIsTouchReleased, ADDRESS_TOUCHSCREEN_IS_RELEASED);

#undef ADD_HOOK
  }

#define DEFINE_INPUT_HOOK(FuncName)                         \
static bool Hook##FuncName(void *pDevice, u32 key, u8 channel) { \
if (DeviceHookContext::GetInstance().use_redirection &&       \
channel != Device::kCustomChannel)                  \
return false;                                         \
\
return HookManager::Call<bool>(HookID::FuncName, pDevice, key, channel);\
}

  DEFINE_INPUT_HOOK(kIsKeyPressed)
  DEFINE_INPUT_HOOK(kIsKeyReleased)
  DEFINE_INPUT_HOOK(kIsKeyDown)
  DEFINE_INPUT_HOOK(kIsKeyRepeated)
  DEFINE_INPUT_HOOK(kIsDPadDown)
  DEFINE_INPUT_HOOK(kIsDPadRepeated)

#undef DEFINE_INPUT_HOOK

#define DEFINE_TOUCH_HOOK(FuncName)                   \
    static bool Hook##FuncName(void *pTouch, u8 channel) {     \
    if (DeviceHookContext::GetInstance().use_redirection && \
    channel != Device::kCustomChannel)            \
    return false;                                   \
    \
    return HookManager::Call<bool>(HookID::FuncName, pTouch, channel);        \
    }

  DEFINE_TOUCH_HOOK(kIsTouchDown)
  DEFINE_TOUCH_HOOK(kIsTouchReleased)

#undef DEFINE_TOUCH_HOOK
};
} // namespace feature