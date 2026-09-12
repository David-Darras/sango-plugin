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
#include "feature/core/hook_manager.h"
#include "game/constant/weather.h"
#include "game/overworld/weather_manager.h"

namespace feature {
enum class WeatherMode : u8 {
  kNormal = 0,
  kToxic = 1,
  kRadioactive = 2,
};

struct WeatherManager {
  MAKE_SINGLETON(WeatherManager)
  WeatherMode mode = WeatherMode::kNormal;
  bool enlarge_drops = true;
  bool keep_weather_indoors = true;
  bool ignore_zone_weather = false;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateZoneWeather,
                            ADDRESS_UPDATE_ZONE_WEATHER,
                            (uptr)UpdateZoneWeatherHook);
    HookManager::Initialize(HookID::kUpdateAreaWeather,
                            ADDRESS_UPDATE_AREA_WEATHER,
                            (uptr)UpdateAreaWeatherHook);
  }

private:
  static u8 PickWeather(overworld::WeatherManager* manager, u8 weather) {
    auto& feat = GetInstance();
    const bool indoors = weather == static_cast<u8>(OverworldWeather::kNone);
    if (feat.ignore_zone_weather || (indoors && feat.keep_weather_indoors)) {
      return manager->GetRequestedWeather();
    }
    return weather;
  }

  static void UpdateZoneWeatherHook(overworld::WeatherManager* self,
                                    u16 zone_id, u8 weather) {
    HookManager::Call<void>(HookID::kUpdateZoneWeather, self, zone_id,
                            PickWeather(self, weather));
  }

  static void UpdateAreaWeatherHook(overworld::WeatherManager* self,
                                    u16 zone_id, u8 weather) {
    HookManager::Call<void>(HookID::kUpdateAreaWeather, self, zone_id,
                            PickWeather(self, weather));
  }
};
} // namespace feature
