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

#include "utils.h"
#include "feature/overworld/feature_overworld_model.h"
#include "game/constant/weather.h"
#include "game/overworld/weather_manager.h"

namespace kaizo {
#define NUM_OVERWORLD_WEATHERS 9

typedef struct {
  OverworldWeather weather;
  u8 weight;
} WeatherTransition;

typedef struct {
  const WeatherTransition* transitions;
  u8 numTransitions;
  u32 minDurationSec;
  u32 maxDurationSec;
} WeatherStateData;

static const WeatherTransition sTrans_Sunny[] = {
    {OverworldWeather::kSunny, 50},
    {OverworldWeather::kCloudy, 35},
    {OverworldWeather::kDry, 15},
};

static const WeatherTransition sTrans_Cloudy[] = {
    {OverworldWeather::kCloudy, 30},
    {OverworldWeather::kRainy, 25},
    {OverworldWeather::kSunny, 20},
    {OverworldWeather::kMisty, 15},
    {OverworldWeather::kSandstorm, 10},
};

static const WeatherTransition sTrans_Rainy[] = {
    {OverworldWeather::kRainy, 35},
    {OverworldWeather::kThunderstorm, 30},
    {OverworldWeather::kCloudy, 25},
    {OverworldWeather::kMisty, 10},
};

static const WeatherTransition sTrans_Thunderstorm[] = {
    {OverworldWeather::kThunderstorm, 30},
    {OverworldWeather::kStormy, 40},
    {OverworldWeather::kRainy, 30},
};

static const WeatherTransition sTrans_Stormy[] = {
    {OverworldWeather::kStormy, 30},
    {OverworldWeather::kRainy, 45},
    {OverworldWeather::kCloudy, 25},
};

static const WeatherTransition sTrans_Misty[] = {
    {OverworldWeather::kMisty, 40},
    {OverworldWeather::kCloudy, 40},
    {OverworldWeather::kSunny, 20},
};

static const WeatherTransition sTrans_Ash[] = {
    {OverworldWeather::kAsh, 40},
    {OverworldWeather::kSandstorm, 30},
    {OverworldWeather::kDry, 20},
    {OverworldWeather::kCloudy, 10},
};

static const WeatherTransition sTrans_Sandstorm[] = {
    {OverworldWeather::kSandstorm, 35},
    {OverworldWeather::kDry, 30},
    {OverworldWeather::kAsh, 20},
    {OverworldWeather::kCloudy, 15},
};

static const WeatherTransition sTrans_Dry[] = {
    {OverworldWeather::kDry, 35},
    {OverworldWeather::kSunny, 35},
    {OverworldWeather::kAsh, 15},
    {OverworldWeather::kSandstorm, 15},
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

static OverworldWeather sCurrentOverworldWeather;
static s64 sWeatherStartTime;
static u32 sCurrentWeatherDuration;

static u32 RollWeatherDuration(OverworldWeather weather) {
  const WeatherStateData* data =
      &sWeatherStateData[static_cast<u8>(weather)];
  u32 range = data->maxDurationSec - data->minDurationSec;
  u32 result = data->minDurationSec + (
                 range ? Utils::GetRandomValue(range) : 0);
  return result >> 5;
}

static OverworldWeather PickNextWeather(OverworldWeather currentWeather) {
  const WeatherStateData* data =
      &sWeatherStateData[static_cast<u8>(currentWeather)];
  u32 totalWeight = 0;
  u32 i;

  for (i = 0; i < data->numTransitions; i++)
    totalWeight += data->transitions[i].weight;

  u32 roll = Utils::GetRandomValue(totalWeight);
  u32 cumulative = 0;

  for (i = 0; i < data->numTransitions; i++) {
    cumulative += data->transitions[i].weight;
    if (roll < cumulative)
      return data->transitions[i].weather;
  }

  return data->transitions[data->numTransitions - 1].weather;
}

void InitializeOverworldWeather() {
  // Don't update weather with in game functions
  ARM_NOP(ADDRESS_UPDATE_AREA_WEATHER + 0xC);
  ARM_NOP(ADDRESS_UPDATE_ZONE_WEATHER + 0x4);

  sCurrentOverworldWeather = OverworldWeather::kSunny;
  Utils::GetElapsedTime(&sWeatherStartTime);
  sCurrentWeatherDuration = RollWeatherDuration(sCurrentOverworldWeather);
}

void UpdateOverworldWeather() {
  static s32 counter = 0;
  counter++;
  if (counter <= 10) return;
  counter = 0;

  s64 now;
  Utils::GetElapsedTime(&now);
  s64 delta = now - sWeatherStartTime;
  u32 elapsedSec = Utils::ConvertTimeToSeconds(&delta);

  if (elapsedSec >= sCurrentWeatherDuration) {
    OverworldWeather next = PickNextWeather(sCurrentOverworldWeather);

    sCurrentOverworldWeather = next;
    sWeatherStartTime = now;
    sCurrentWeatherDuration = RollWeatherDuration(next);

    overworld::WeatherManager::GetInstance().SetWeather(static_cast<u8>(next));
  }

  overworld::WeatherManager::GetInstance().SetWeather(
      static_cast<u8>(sCurrentOverworldWeather));
}
}