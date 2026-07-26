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
#include "feature_app.h"
#include "feature_camera.h"
#include "feature_light.h"
#include "feature_nuzlocke.h"
#include "feature_overworld_model.h"
#include "feature/hook_manager.h"
#include "game/constant/map.h"
#include "game/constant/weather.h"
#include "game/overworld/weather_manager.h"
#include "ui/log_application.h"

namespace feature {
struct Overworld {
  MAKE_SINGLETON(Overworld)
  u16 background_music = 31;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGetOverworldBackgroundMusic,
                            ADDRESS_GET_OVERWORLD_BACKGROUND_MUSIC,
                            (uptr)GetBackgroundMusic);
    HookManager::Initialize(HookID::kSetCulling,
                            0x00779E64,
                            (uptr)SetCullingHook, false);
  }

  STATIC_INLINE void Patch() {
    Nuzlocke::FixNickname();
    Nuzlocke::FixConfig();
    HookManager::Enable(HookID::kEncounterSetPokemon);
    HookManager::ForceEnable(HookID::kSetCulling);
    HookManager::ForceEnable(HookID::kCheckAppRequest);
    auto& team = savedata::PokemonTeam::GetInstance();
    team.HealAllPokemons();

    // Simulate a button press
    WRITE(vu32, 0x00715C48, 0xE1A00000);
  }

  static u32
  GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form) {
    // u32 result = HookManager::Call<u32>(HookID::kGetOverworldBackgroundMusic,
    //                                     sound_manager, map_id, player_form);
    return (1 << 16) + GetInstance().background_music; // NO SOUND
  }

  static void SetCullingHook(void* model, bool use_culling) {
    use_culling = false;
    HookManager::Call<void>(HookID::kSetCulling, model, use_culling);
  }
};
} // namespace feature