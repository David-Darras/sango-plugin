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

#include "core/utils.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/patch/weather_override.h"
#include "overworld/constant/weather.h"
#include "overworld/native/weather_manager.h"

namespace kaizo {
using overworld::Weather;

#define NUM_OVERWORLD_WEATHERS 9

typedef struct {
  Weather weather;
  u8 weight;
} WeatherTransition;

typedef struct {
  const WeatherTransition* transitions;
  u8 numTransitions;
  u32 minDurationSec;
  u32 maxDurationSec;
} WeatherStateData;

static const WeatherTransition sTrans_Sunny[] = {
    {Weather::kSunny, 50},
    {Weather::kCloudy, 35},
    {Weather::kDry, 15},
};

static const WeatherTransition sTrans_Cloudy[] = {
    {Weather::kCloudy, 30},
    {Weather::kRainy, 25},
    {Weather::kSunny, 20},
    {Weather::kMisty, 15},
    {Weather::kSandstorm, 10},
};

static const WeatherTransition sTrans_Rainy[] = {
    {Weather::kRainy, 35},
    {Weather::kThunderstorm, 30},
    {Weather::kCloudy, 25},
    {Weather::kMisty, 10},
};

static const WeatherTransition sTrans_Thunderstorm[] = {
    {Weather::kThunderstorm, 30},
    {Weather::kStormy, 40},
    {Weather::kRainy, 30},
};

static const WeatherTransition sTrans_Stormy[] = {
    {Weather::kStormy, 30},
    {Weather::kRainy, 45},
    {Weather::kCloudy, 25},
};

static const WeatherTransition sTrans_Misty[] = {
    {Weather::kMisty, 40},
    {Weather::kCloudy, 40},
    {Weather::kSunny, 20},
};

static const WeatherTransition sTrans_Ash[] = {
    {Weather::kAsh, 40},
    {Weather::kSandstorm, 30},
    {Weather::kDry, 20},
    {Weather::kCloudy, 10},
};

static const WeatherTransition sTrans_Sandstorm[] = {
    {Weather::kSandstorm, 35},
    {Weather::kDry, 30},
    {Weather::kAsh, 20},
    {Weather::kCloudy, 15},
};

static const WeatherTransition sTrans_Dry[] = {
    {Weather::kDry, 35},
    {Weather::kSunny, 35},
    {Weather::kAsh, 15},
    {Weather::kSandstorm, 15},
};

#define WEATHER_ENTRY(arr, tmin, tmax) \
{ arr, SIZE(arr), tmin, tmax }

static const WeatherStateData sWeatherStateData[NUM_OVERWORLD_WEATHERS] =
{
    WEATHER_ENTRY(sTrans_Sunny, 180, 600), // 0 SUNNY
    WEATHER_ENTRY(sTrans_Rainy, 120, 400), // 1 RAINY
    WEATHER_ENTRY(sTrans_Thunderstorm, 90, 300), // 2 THUNDERSTORM
    WEATHER_ENTRY(sTrans_Misty, 150, 350), // 3 MISTY
    WEATHER_ENTRY(sTrans_Ash, 200, 500), // 4 ASH
    WEATHER_ENTRY(sTrans_Sandstorm, 150, 450), // 5 SANDSTORM
    WEATHER_ENTRY(sTrans_Cloudy, 100, 300), // 6 CLOUDY
    WEATHER_ENTRY(sTrans_Stormy, 60, 240), // 7 STORMY
    WEATHER_ENTRY(sTrans_Dry, 200, 600), // 8 DRY
};

static Weather sCurrentOverworldWeather;
static s64 sWeatherStartTime;
static u32 sCurrentWeatherDuration;

static u32 RollWeatherDuration(Weather weather) {
  const WeatherStateData* data =
      &sWeatherStateData[static_cast<u8>(weather)];
  u32 range = data->maxDurationSec - data->minDurationSec;
  u32 result = data->minDurationSec + (
                 range ? core::Utils::GetRandomValue(range) : 0);
  return result >> 5;
}

static Weather PickNextWeather(Weather currentWeather) {
  const WeatherStateData* data =
      &sWeatherStateData[static_cast<u8>(currentWeather)];
  u32 totalWeight = 0;
  u32 i;

  for (i = 0; i < data->numTransitions; i++)
    totalWeight += data->transitions[i].weight;

  u32 roll = core::Utils::GetRandomValue(totalWeight);
  u32 cumulative = 0;

  for (i = 0; i < data->numTransitions; i++) {
    cumulative += data->transitions[i].weight;
    if (roll < cumulative)
      return data->transitions[i].weather;
  }

  return data->transitions[data->numTransitions - 1].weather;
}

void InitializeOverworldWeather() {
  // The weather below drives the game, zones never do.
  overworld::WeatherOverride::GetInstance().ignore_zone_weather = true;

  sCurrentOverworldWeather = Weather::kSunny;
  core::Utils::GetElapsedTime(&sWeatherStartTime);
  sCurrentWeatherDuration = RollWeatherDuration(sCurrentOverworldWeather);
}

void UpdateOverworldWeather() {
  static s32 counter = 0;
  counter++;
  if (counter <= 10) return;
  counter = 0;

  s64 now;
  core::Utils::GetElapsedTime(&now);
  s64 delta = now - sWeatherStartTime;
  u32 elapsedSec = core::Utils::ConvertTimeToSeconds(&delta);

  if (elapsedSec >= sCurrentWeatherDuration) {
    Weather next = PickNextWeather(sCurrentOverworldWeather);

    sCurrentOverworldWeather = next;
    sWeatherStartTime = now;
    sCurrentWeatherDuration = RollWeatherDuration(next);

    overworld::WeatherManager::GetInstance().SetWeather(next);
  }

  overworld::WeatherManager::GetInstance().SetWeather(sCurrentOverworldWeather);
}
}