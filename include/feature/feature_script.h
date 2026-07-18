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
#include "hook_manager.h"
#include "system/file.h"
#include "ui/log_application.h"
#include "game/script/amx.h"

namespace feature {
class Script {
  MAKE_SINGLETON(Script)

public:
  STATIC_INLINE void Initialize() {
    // HookManager::Initialize(HookID::kLoadScript, 0x003AB0C8, (uptr)LoadScriptHook);
    HookManager::Initialize(HookID::kInitializeVirtualMachine, 0x00505DB4,
                            (uptr)InitializeVirtualMachineHook);
  }

  static void InitializeVirtualMachineHook(script::AmxRuntime* runtime,
                                           script::AmxHeader* header) {
    HookManager::Call<void>(HookID::kInitializeVirtualMachine, runtime, header);
    void* buffer = (void*)header;
    u32 size = header->stack_top_offset;
    // ui::LogApplication::Print(u"Run Script %08X", buffer);
    {
      c16 filename[BUFFER_SIZE];
      Utils::Format(filename, u"sdmc:/script_%08X.amx", buffer);
      File file(filename, true);
      file.Write(buffer, size);
    }
  }
};
}