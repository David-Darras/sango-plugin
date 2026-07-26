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
#include "feature_camera.h"
#include "feature_encounter.h"
#include "feature_engine.h"
#include "feature_light.h"
#include "feature_pokemon_model.h"
#include "feature_title_screen.h"
#include "game/event_manager.h"
#include "game/process_manager.h"
#include "game/constant/weather.h"
#include "game/overworld/map_manager.h"
#include "system/sound.h"

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
  bool is_introduction = false;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kMainProcessLoop, 0x003AB62C,
                            (uptr)MainProcessLoopHook);
  }

  static u32 MainProcessLoopHook(game::ProcessManager* manager) {
    struct Starter {
      PokeInfo info;
      u8 padding[0x54 - sizeof(PokeInfo)];
    }* starter = (Starter*)(0x0804F3F0);
    auto& replace_pokemon_model = feature::PokemonModel::GetInstance().
        is_enabled;

    if (manager->handle_ != nullptr) {
      auto* process = manager->handle_->process_;
      if (process != nullptr) {
        uptr vtable = (uptr)process->vtable;
        auto state = manager->handle_->state_;
        if (state == game::ProcessState::kLoading)
          switch (vtable) {
            case ADDRESS_INTRODUCTION_VTABLE:
            case ADDRESS_CINEMATIC_VTABLE:
              replace_pokemon_model = true;
              break;
            case ADDRESS_TITLE_SCREEN_VTABLE:
              replace_pokemon_model = true;
              TitleScreen::Initialize();
              break;
            case ADDRESS_SELECT_STARTER_VTABLE:
              starter[0].info.is_egg = true;
              starter[0].info.species = 0;
              starter[1].info.is_egg = true;
              starter[1].info.species = 0;
              starter[2].info.is_egg = true;
              starter[2].info.species = 0;
              break;
            case ADDRESS_BATTLE_VTABLE:
              replace_pokemon_model = false;
              HookManager::ForceEnable(HookID::kUpdateBattleView);
              break;
            case ADDRESS_OVERWORLD_VTABLE:
              replace_pokemon_model = false;
              Nuzlocke::FixNickname();
              Nuzlocke::FixConfig();
              HookManager::Enable(HookID::kEncounterSetPokemon);
              break;
            default:
              HookManager::Clear(HookID::kEncounterSetPokemon);
              break;
          }
      }
    }
    return HookManager::Call<u32>(HookID::kMainProcessLoop, manager);
  }

  static void OnEnterOverworld() {
    GameApp::OnEnterOverworld();
    auto& heal_team = GetInstance().heal_team;
    if (heal_team) {
      auto& team = savedata::PokemonTeam::GetInstance();
      team.HealAllPokemons();
      heal_team = false;
    }
  }

  static void OnUpdateMap(u16 map_id) {
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
    GetInstance().heal_team = true;
    // feature::Engine::GetInstance().game_speed = 1;
#endif
  }

  static void OnEnterBattle() {
#if ENABLE_NUZLOCKE_FEATURES == 1

    // Disable shaders
    // ARM_RET(0x003989B0);
    // ARM_RET(0x003881EC);

    HookManager::ForceEnable(HookID::kUpdateExp);
    HookManager::ForceEnable(HookID::kStartMegaEvolveAnimation);
    HookManager::ForceEnable(HookID::kStartBattleAnimation);
    HookManager::ForceEnable(HookID::kPlayBattleAnimation);

    // CAN'T SAVE
    // feature::Engine::GetInstance().game_speed = 2;

    // Only access to pokeball
    WRITE(vu8, 0x007CB09C, 2); // HP/PP -> Ball
    // WRITE(vu8, 0x007CB0B4, 2) // Ball
    WRITE(vu8, 0x007CB0CC, 2); // Status -> Ball
    WRITE(vu8, 0x007CB0E4, 2); // Battle -> Item

    // Disable master ball feature
    ARM_NOP(0x007227A4);
    ARM_NO_COND(0x007227B8);

    // same ratio for all balls
    WRITE(vu32, 0x007232E4, 0xE3A00A01); // mov r0, #0x1000
    ARM_RET(0x007232E8);

#endif
  }

  static void OnUpdateBattle() {
  }

  static void OnExitOverworld() {
  }

  static void OnEnterAppStatus() {
  }

  static void OnExitAppStatus() {
  }

  static void OnUpdateAppStatus() {
    GameApp::OnUpdateAppStatus();
  }

  static void DoEachFrame() {
    auto& ctx = GetInstance();

    uptr vtable = 0;
    ctx.current_process = game::ProcessManager::GetInstance().
        GetCurrentProcessName(vtable);

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
          OnExitAppStatus();
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