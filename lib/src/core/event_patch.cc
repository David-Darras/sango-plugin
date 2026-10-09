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
 * @file event_patch.cc
 * @brief Calls the patches of the game events when they start.
 *
 * The declarations are in core/patch/event_patch.h.
 */

#include "core/patch/event_patch.h"
#include "core/patch/script_loader.h"
#include "core/hook.h"
#include "core/native/event_manager.h"

namespace core {

namespace {
core::Hook<u32(EventManager*)> main_event_loop_hook;
} // namespace

void EventPatch::Initialize() {
  main_event_loop_hook.Install(sys::address::kMainEventLoop, MainEventLoopHook);
}

u32 EventPatch::MainEventLoopHook(EventManager* manager) {
  manager->Patch(OnLoad, OnUpdate);
  return main_event_loop_hook(manager);
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
