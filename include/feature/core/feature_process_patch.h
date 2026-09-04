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

#include "feature/core/feature_app.h"
#include "feature/ui/feature_app_status.h"
#include "feature/battle/feature_battle.h"
#include "feature/ui/feature_keyboard.h"
#include "feature/overworld/feature_overworld.h"
#include "feature/ui/feature_title_screen.h"
#include "game/core/process_manager.h"

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
        AppStatus::PatchUpdate();
        break;
      case ADDRESS_BATTLE_VTABLE:
        Battle::PatchUpdate();
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
    if (vtable != ADDRESS_KEYBOARD_VTABLE) {
      Keyboard::GetInstance().is_opened = false;
    }

    switch (vtable) {
#ifdef KAIZO
      case ADDRESS_INTRODUCTION_VTABLE:
      case ADDRESS_CINEMATIC_VTABLE:
        kaizo::ShouldReplacePokemonModel(true);
        break;
      case ADDRESS_SELECT_STARTER_VTABLE:
        kaizo::PatchStarterView();
        break;
#endif
      case ADDRESS_TITLE_SCREEN_VTABLE:
        TitleScreen::PatchLoad();
        break;
      case ADDRESS_BATTLE_VTABLE:
        Battle::PatchLoad();
        break;
      case ADDRESS_OVERWORLD_VTABLE:
        Overworld::PatchLoad();
        break;
      case ADDRESS_APP_STATUS_VTABLE:
        AppStatus::PatchLoad();
        break;
      case ADDRESS_KEYBOARD_VTABLE:
        Keyboard::PatchLoad();
        break;
      default:
        break;
    }
  }
};
} // namespace feature


#endif //SANGO_PLUGIN_FEATURE_PROCESS_H