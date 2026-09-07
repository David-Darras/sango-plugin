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

#include "feature/battle/feature_battle.h"
#include "feature/core/feature_transition_logger.h"
#include "feature/overworld/feature_overworld.h"
#include "game/core/event_manager.h"

namespace feature {
class EventPatch {
  MAKE_SINGLETON(EventPatch)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kMainEventLoop, ADDRESS_MAIN_EVENT_LOOP,
                            (uptr)MainEventLoopHook);
  }

  static u32 MainEventLoopHook(game::EventManager* manager) {
    manager->Patch(OnLoad, OnUpdate);
    return HookManager::Call<u32>(HookID::kMainEventLoop, manager);
  }

  static void OnUpdate(uptr vtable) {
  }

  static void OnLoad(uptr vtable) {
    switch (vtable) {
      case ADDRESS_OVERWORLD_CALL_SCRIPT_VTABLE:
        GiftPokemon::PatchLoad();
        StaticEncounter::PatchLoad();
        break;
    }
  }
};
} // namespace feature