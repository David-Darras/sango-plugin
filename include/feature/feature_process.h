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
#include "game/process_manager.h"
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
    auto& heal_team = GetInstance().heal_team;
    if (heal_team) {
#if ENABLE_NUZLOCKE_MENU == 1
      auto& team = savedata::PokemonTeam::GetInstance();
      for (u32 i = 0; i < team.count; i++) {
        team.pokemons[i]->accessor->Decrypt();
        team.pokemons[i]->runtime->hp = team.pokemons[i]->runtime->max_hp;
        team.pokemons[i]->accessor->Encrypt();
      }
#endif
      heal_team = false;
    }
  }

  static void OnUpdateOverworld() {
  }

  static void OnExitBattle() {
    GetInstance().heal_team = true;
  }

  static void OnEnterBattle() {
  }

  static void OnUpdateBattle() {
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