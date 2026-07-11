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
#include "game/overworld/weather_manager.h"
#include "feature_app.h"
#include "feature_encounter.h"
#include "feature_engine.h"
#include "game/process_manager.h"
#include "game/constant/weather.h"
#include "game/overworld/map_manager.h"

namespace feature {
class ProcessHookContext {
  MAKE_SINGLETON(ProcessHookContext)
public:
  enum State {
    kUnknow,
    kOverworld,
    kBattle,
    kAppStatus,
  };

  const char* current_process = nullptr;
  State old_state = kUnknow;
  State current_state = kUnknow;
  bool heal_team = false;

  static void OnEnterOverworld() {
    AppHookContext::OnEnterOverworld();
#if ENABLE_NUZLOCKE_FEATURES == 1
    auto& heal_team = GetInstance().heal_team;
    if (heal_team) {
      auto& team = savedata::PokemonTeam::GetInstance();
      team.HealAllPokemons();
      heal_team = false;
    }
#endif
  }

  static void OnUpdateMap(u16 id) {
    auto& weather_manager = overworld::WeatherManager::GetInstance();
    u8& request = weather_manager.GetRequestedWeather();
    switch (id) {
      case MAP_ROUTE_101:
        request = WEATHER_RAIN;
        break;
      case MAP_ROUTE_102:
        request = WEATHER_SANDSTORM;
        break;
      case MAP_ROUTE_103:
        request = WEATHER_HAIL;
        break;
    }
  }

  static void OnUpdateOverworld() {
    static u32 last_map_id = 0;
    u32& map_id = overworld::MapManager::GetInstance().GetMapId();
    if (map_id != last_map_id) {
      last_map_id = map_id;
      OnUpdateMap(last_map_id);
    }
  }

  static void OnExitBattle() {
#if ENABLE_NUZLOCKE_FEATURES == 1
    // GetInstance().heal_team = true;
    feature::EngineHookContext::GetInstance().game_speed = 1;
#endif
  }

  static void OnEnterBattle() {
#if ENABLE_NUZLOCKE_FEATURES == 1

    // Disable shaders
    // ARM_RET(0x003989B0);
    // ARM_RET(0x003881EC);

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
  }

  static void OnExitOverworld() {
  }

  static void OnEnterAppStatus() {
    AppHookContext::OnEnterAppStatus();
  }

  static void OnExitAppstatus() {
  }

  static void OnUpdateAppStatus() {
    AppHookContext::OnUpdateAppStatus();
  }

  static void DoEachFrame() {
    auto& ctx = GetInstance();

    ctx.current_process = game::ProcessManager::GetInstance().
        GetCurrentProcessName();

    bool is_fieldmap = strcmp(ctx.current_process, PROCESS_NAME_FIELD_MAP) == 0;
    bool is_battle = strcmp(ctx.current_process, PROCESS_NAME_BATTLE) == 0;
    bool is_app_status = strcmp(ctx.current_process, PROCESS_NAME_APP_STATUS) ==
                         0;

    ctx.old_state = ctx.current_state;

    if (is_fieldmap) {
      ctx.current_state = kOverworld;
    } else if (is_battle) {
      ctx.current_state = kBattle;
    } else if (is_app_status) {
      ctx.current_state = kAppStatus;
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
        case kAppStatus:
          OnExitAppstatus();
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
        case kAppStatus:
          OnEnterAppStatus();
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
        case kAppStatus:
          OnUpdateAppStatus();
          break;
        default:
          break;
      }
    }
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_PROCESS_H