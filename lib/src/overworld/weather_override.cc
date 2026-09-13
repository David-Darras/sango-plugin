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

#include "overworld/patch/weather_override.h"

#include "core/hook_manager.h"
#include "overworld/native/weather_manager.h"

namespace overworld {

void WeatherOverride::Initialize() {
  core::HookManager::Initialize(HookId::kUpdateZoneWeather,
                          address::kUpdateZoneWeather,
                          (uptr)UpdateZoneWeatherHook);
  core::HookManager::Initialize(HookId::kUpdateAreaWeather,
                          address::kUpdateAreaWeather,
                          (uptr)UpdateAreaWeatherHook);
}

Weather WeatherOverride::PickWeather(WeatherManager* manager,
                                       Weather weather) {
  auto& feat = GetInstance();
  const bool indoors = weather == Weather::kNone;
  if (feat.ignore_zone_weather || (indoors && feat.keep_weather_indoors)) {
    return manager->GetRequestedWeather();
  }
  return weather;
}

void WeatherOverride::UpdateZoneWeatherHook(WeatherManager* self,
                                            u16 zone_id, Weather weather) {
  core::HookManager::Call<void>(HookId::kUpdateZoneWeather, self, zone_id,
                          PickWeather(self, weather));
}

void WeatherOverride::UpdateAreaWeatherHook(WeatherManager* self,
                                            u16 zone_id, Weather weather) {
  core::HookManager::Call<void>(HookId::kUpdateAreaWeather, self, zone_id,
                          PickWeather(self, weather));
}

} // namespace overworld
