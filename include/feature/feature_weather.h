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
#include "feature_nuzlocke.h"
#include "hook_manager.h"
#include "game/overworld/weather_manager.h"

namespace feature {
class WeatherManager {
  MAKE_SINGLETON(WeatherManager)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateZoneWeather,
                            ADDRESS_UPDATE_ZONE_WEATHER,
                            (uptr)UpdateZoneWeatherHook);
    HookManager::Initialize(HookID::kUpdateAreaWeather,
                            ADDRESS_UPDATE_AREA_WEATHER,
                            (uptr)UpdateAreaWeatherHook);
  }

  static void UpdateZoneWeatherHook(overworld::WeatherManager* manager,
                                    u16 map_id,
                                    u8 weather) {
    weather = Nuzlocke::FixAll(map_id, weather);
    // manager->SetMapId(map_id);
    // manager->SetWeather(weather);
  }

  static void UpdateAreaWeatherHook(overworld::WeatherManager* manager,
                                    u16 map_id,
                                    u8 weather) {
    weather = Nuzlocke::FixAll(map_id, weather);
    // return HookManager::Call<void>(HookID::kUpdateAreaWeather, manager, map_id,
    //                                weather);
  }
};
}