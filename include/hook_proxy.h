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

#ifndef SANGO_PLUGIN_HOOK_H
#define SANGO_PLUGIN_HOOK_H

#include <CTRPluginFramework/System/Hook.hpp>

#include "common.h"

class HookProxy {
public:
  HookProxy(u32 target_address, u32 callback_address, bool enable = true) {
    hook_.InitializeForMitm(target_address, callback_address);
    if (enable) Enable();
  }

  FORCE_INLINE void Enable() { hook_.Enable(); }
  FORCE_INLINE void Disable() { hook_.Disable(); }

private:
  CTRPluginFramework::Hook hook_;
};

#endif //SANGO_PLUGIN_HOOK_H