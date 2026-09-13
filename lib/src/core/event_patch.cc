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

#include "core/patch/event_patch.h"
#include "core/patch/script_loader.h"
#include "core/hook_manager.h"
#include "core/native/event_manager.h"

namespace core {

void EventPatch::Initialize() {
  HookManager::Initialize(HookId::kMainEventLoop, sys::address::kMainEventLoop,
                          (uptr)MainEventLoopHook);
}

u32 EventPatch::MainEventLoopHook(EventManager* manager) {
  manager->Patch(OnLoad, OnUpdate);
  return HookManager::Call<u32>(HookId::kMainEventLoop, manager);
}

void EventPatch::OnUpdate(uptr vtable) {
}

void EventPatch::OnLoad(uptr vtable) {
  switch (vtable) {
    case overworld::address::kCallScriptVtable:
      ScriptLoader::PatchLoad();
      break;
  }
}

} // namespace core
