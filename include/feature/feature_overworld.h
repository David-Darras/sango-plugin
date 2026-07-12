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
#include "feature_camera.h"
#include "feature_light.h"
#include "feature_overworld_model.h"
#include "feature/hook_manager.h"
#include "game/constant/map.h"
#include "game/constant/weather.h"
#include "game/overworld/weather_manager.h"
#include "ui/log_application.h"

#define ADDRESS_GET_OVERWORLD_BACKGROUND_MUSIC (0x003C79F8)

namespace feature {
struct Overworld {
  MAKE_SINGLETON(Overworld)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGetOverworldBackgroundMusic,
                            ADDRESS_GET_OVERWORLD_BACKGROUND_MUSIC,
                            (uptr)GetBackgroundMusic);
  }

  static u32
  GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form) {
    u32 result = HookManager::Call<u32>(HookID::kGetOverworldBackgroundMusic,
                                        sound_manager, map_id, player_form);
    ui::LogApplication::Print(u"map=%X", map_id);
    OnUpdateMap(map_id);
    switch (map_id) {
      case MAP_BRENDAN_HOUSE:
      case MAP_BRENDAN_BEDROOM:
      case MAP_MAY_BEDROOM:
      case MAP_MAY_HOUSE:
      case MAP_BIRCH_LABORATORY:
      case MAP_LITTLEROOT_TOWN:
        return (1 << 16) + 40;
      default:
        return result;
    }
  }


  static void OnUpdateMap(u16 id) {
    auto& weather_manager = overworld::WeatherManager::GetInstance();
    auto& weather = weather_manager.GetRequestedWeather();
    auto& camera = Camera::GetInstance();
    auto& light = Light::GetInstance();
    switch (id) {
      case MAP_BRENDAN_HOUSE:
      case MAP_BRENDAN_BEDROOM:
      case MAP_MAY_BEDROOM:
      case MAP_MAY_HOUSE:
      case MAP_BIRCH_LABORATORY:
        camera.SetCameraFree(103, 200, 514, -1.22, -0.54);
        light.SetAmbient(0.4, 0.4, 0.4);
        light.SetDiffuse(0, 0, 0);
        break;
      case MAP_LITTLEROOT_TOWN:
        weather = WEATHER_OVERWORLD_STORMY;
        camera.state = Camera::State::kRotate;
        camera.radius = 400.0f;
        camera.height = 300.0f;
        camera.theta_speed = 0.002f;
        light.SetAmbient(0, 0, 1);
        light.SetDiffuse(0, 0, 1);
        // Sound::PlayPokemonCry(SPECIES_KYOGRE);
        break;
      case MAP_ROUTE_101:
      case MAP_ROUTE_102:
      case MAP_ROUTE_103:
      case MAP_OLDALE_TOWN:
      default:
        camera.state = Camera::State::kIdle;
        light.ResetAmbient();
        light.ResetDiffuse();
        break;
    }
  }
};
} // namespace feature