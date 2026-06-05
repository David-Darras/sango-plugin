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
#include "feature_engine.h"
#include "game/process_manager.h"
#include "game/battle/manager.h"
#include "game/global_data/pokemon.h"
#include "ui/log_application.h"

namespace feature {
class ProcessHookContext {
  MAKE_SINGLETON(ProcessHookContext)
public:
  enum State {
    kUnknow,
    kOverworld,
    kBattle,
  };

  const char* current_process = nullptr;
  State old_state = kUnknow;
  State current_state = kUnknow;
  bool heal_team = false;

  static void OnEnterOverworld() {
    AppHookContext::OnEnterOverworld();
#if ENABLE_NUZLOCKE_MENU == 1
    auto& heal_team = GetInstance().heal_team;
    if (heal_team) {
      auto& team = savedata::PokemonTeam::GetInstance();
      team.HealAllPokemons();
      heal_team = false;
    }
#endif
  }

  static void OnUpdateOverworld() {
  }

  static void OnExitBattle() {
#if ENABLE_NUZLOCKE_MENU == 1
    GetInstance().heal_team = true;
    feature::EngineHookContext::GetInstance().game_speed = 1;
#endif
  }

  static void OnEnterBattle() {
#if ENABLE_NUZLOCKE_MENU == 1
    HookManager::ForceEnable(HookID::kUpdateExp);

    feature::EngineHookContext::GetInstance().game_speed = 2;

    // Only access to pokeball
    WRITE(vu8, 0x007CB09C, 2); // HP/PP -> Ball
    // WRITE(vu8, 0x007CB0B4, 2) // Ball
    WRITE(vu8, 0x007CB0CC, 2); // Status -> Ball
    WRITE(vu8, 0x007CB0E4, 2); // Battle -> Item

#endif
  }

  static void OnUpdateBattle() {
#if ENABLE_NUZLOCKE_MENU == 1
    for (u32 i = 0; i < 6; i++) {
      u32 pkmMdl = *(u32*)(0x83F84C0 + 4 * i); // Base address of the 3D model
      if (pkmMdl == 0)
        continue;

      u16 pkmNum = *(u16*)(pkmMdl + 0x170); // Pokédex number
      if (pkmNum >= 722)
        continue;

      u32 pkmData = *(u32*)0x617A00 + 0x50 * pkmNum;
      // Personal data of the Pokémon
      float realSize = (float)*(u16*)(pkmData + 0x24);
      // Actual height according to the Pokédex
      float defaultSize = (float)*(u16*)(pkmData + 0x3C);
      // Default displayed size in battle
      float ratio = realSize / defaultSize;

      // Update Pokémon scale
      *(float*)(pkmMdl + 0x34) = ratio;
      *(float*)(pkmMdl + 0x38) = ratio;
      *(float*)(pkmMdl + 0x3C) = ratio;

      // Mark the model to be updated
      *(bool*)(pkmMdl + 0x4C) = true;
    }
#endif
  }

  static void OnExitOverworld() {
  }

  static void DoEachFrame() {
    auto& ctx = GetInstance();

    ctx.current_process = game::ProcessManager::GetInstance().
        GetCurrentProcessName();

    bool is_fieldmap = strcmp(ctx.current_process, PROCESS_NAME_FIELD_MAP) == 0;
    bool is_battle = strcmp(ctx.current_process, PROCESS_NAME_BATTLE) == 0;

    ctx.old_state = ctx.current_state;

    if (is_fieldmap) {
      ctx.current_state = kOverworld;
    } else if (is_battle) {
      ctx.current_state = kBattle;
    } else {
      ctx.current_state = kUnknow;
    }

    if (ctx.current_state != ctx.old_state) {
      switch (ctx.old_state) {
        case kOverworld:
          OnExitOverworld();
          break;
        case kBattle:
          OnExitBattle();
          break;
        default:
          break;
      }

      switch (ctx.current_state) {
        case kOverworld:
          OnEnterOverworld();
          break;
        case kBattle:
          OnEnterBattle();
          break;
        default:
          break;
      }
    } else {
      switch (ctx.current_state) {
        case kOverworld:
          OnUpdateOverworld();
          break;
        case kBattle:
          OnUpdateBattle();
          break;
        default:
          break;
      }
    }
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_PROCESS_H