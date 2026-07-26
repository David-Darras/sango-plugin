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

#ifndef SANGO_PLUGIN_FEATURE_PROCESS_H
#define SANGO_PLUGIN_FEATURE_PROCESS_H

#include "feature_app.h"
#include "feature_battle.h"
#include "feature_title_screen.h"
#include "game/process_manager.h"

namespace feature {
class ProcessPatch {
  MAKE_SINGLETON(ProcessPatch)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kMainProcessLoop, ADDRESS_MAIN_PROCESS_LOOP,
                            (uptr)MainProcessLoopHook);
  }

  static u32 MainProcessLoopHook(game::ProcessManager* manager) {
    manager->Patch(OnLoad, OnUpdate);
    return HookManager::Call<u32>(HookID::kMainProcessLoop, manager);
  }

  static void OnUpdate(uptr vtable) {
    switch (vtable) {
      case ADDRESS_APP_STATUS_VTABLE:
        GameApp::PatchAppStatus();
        break;
      default:
        break;
    }
  }

  static void OnLoad(uptr vtable) {
#ifdef KAIZO
    kaizo::ShouldReplacePokemonModel(false);
#endif

    if (vtable != ADDRESS_OVERWORLD_VTABLE) {
      HookManager::Clear(HookID::kGetEncounterPokemon);
    }

    switch (vtable) {
      case ADDRESS_INTRODUCTION_VTABLE:
      case ADDRESS_CINEMATIC_VTABLE:
#ifdef KAIZO
        kaizo::ShouldReplacePokemonModel(true);
#endif
        break;
      case ADDRESS_TITLE_SCREEN_VTABLE:
        TitleScreen::Patch();
        break;
      case ADDRESS_SELECT_STARTER_VTABLE:
#ifdef KAIZO
        kaizo::PatchStarterView();
#endif
        break;
      case ADDRESS_BATTLE_VTABLE:
        Battle::Patch();
        break;
      case ADDRESS_OVERWORLD_VTABLE:
        Overworld::Patch();
        break;
      default:
        break;
    }
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_PROCESS_H