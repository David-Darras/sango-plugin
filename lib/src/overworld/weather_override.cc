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

/**
 * @file weather_override.cc
 * @brief Changes the weather of the overworld.
 *
 * The declarations are in overworld/patch/weather_override.h.
 */

#include "overworld/patch/weather_override.h"

#include "core/hook.h"
#include "overworld/native/weather_manager.h"

namespace overworld {

namespace {
core::Hook<void(WeatherManager*, u16, u32, u32, u32)> set_zone_hook; ///< XY.
core::Hook<void(WeatherManager*, u16, Weather)> update_zone_weather_hook;
core::Hook<void(WeatherManager*, u16, Weather)> update_area_weather_hook;
} // namespace

void WeatherOverride::Initialize() {
#ifdef GAME_XY
  set_zone_hook.Install(address::kWeatherSetZone, SetZoneHook);
#else
  update_zone_weather_hook.Install(address::kUpdateZoneWeather,
                                   UpdateZoneWeatherHook);
  update_area_weather_hook.Install(address::kUpdateAreaWeather,
                                   UpdateAreaWeatherHook);
#endif
}

#ifdef GAME_XY
void WeatherOverride::SetZoneHook(WeatherManager* self, u16 zone_id,
                                  u32 list_idx, u32 wind_id, u32 arg4) {
  auto& feat = GetInstance();
  const bool indoors = list_idx == WeatherManager::kNoWeatherZone;
  if (feat.ignore_zone_weather || self->IsForced() ||
      (indoors && feat.keep_weather_indoors)) {
    list_idx = self->GetZoneListIndex();
  }
  set_zone_hook(self, zone_id, list_idx, wind_id, arg4);
}
#endif

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
  update_zone_weather_hook(self, zone_id, PickWeather(self, weather));
}

void WeatherOverride::UpdateAreaWeatherHook(WeatherManager* self,
                                            u16 zone_id, Weather weather) {
  update_area_weather_hook(self, zone_id, PickWeather(self, weather));
}

} // namespace overworld
