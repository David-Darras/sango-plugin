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

#include <type_traits>

#include "common.h"
#include "overworld/constant/weather.h"
#include "overworld/constant/weather_mode.h"

namespace overworld {
class WeatherManager;

struct WeatherOverrideSettings {
  WeatherMode mode = WeatherMode::kNormal;
  bool enlarge_drops = true;
  bool keep_weather_indoors = true;
  bool ignore_zone_weather = false;
};
static_assert(std::is_standard_layout<WeatherOverrideSettings>::value,
              "WeatherOverrideSettings must have standard layout");

/// Decides which weather the field really shows when a zone or an area asks
/// for one: keeps the current weather indoors, or ignores zones altogether so
/// a product can drive the weather itself.
struct WeatherOverride : public WeatherOverrideSettings {
  MAKE_SINGLETON(WeatherOverride)

  static void Initialize();

private:
  static Weather PickWeather(WeatherManager* manager, Weather weather);
  static void UpdateZoneWeatherHook(WeatherManager* self, u16 zone_id,
                                    Weather weather);
  static void UpdateAreaWeatherHook(WeatherManager* self, u16 zone_id,
                                    Weather weather);
};
} // namespace overworld
