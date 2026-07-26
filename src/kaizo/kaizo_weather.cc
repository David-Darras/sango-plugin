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
#include "feature/feature_overworld_model.h"
#include "game/constant/weather.h"
#include "game/overworld/weather_manager.h"

namespace kaizo {
#define NUM_OVERWORLD_WEATHERS 9

typedef struct {
  u8 weather;
  u8 weight;
} WeatherTransition;

typedef struct {
  const WeatherTransition* transitions;
  u8 numTransitions;
  u32 minDurationSec;
  u32 maxDurationSec;
} WeatherStateData;

static const WeatherTransition sTrans_Sunny[] = {
    {WEATHER_OVERWORLD_SUNNY, 50},
    {WEATHER_OVERWORLD_CLOUDY, 35},
    {WEATHER_OVERWORLD_DRY, 15},
};

static const WeatherTransition sTrans_Cloudy[] = {
    {WEATHER_OVERWORLD_CLOUDY, 30},
    {WEATHER_OVERWORLD_RAINY, 25},
    {WEATHER_OVERWORLD_SUNNY, 20},
    {WEATHER_OVERWORLD_MISTY, 15},
    {WEATHER_OVERWORLD_SANDSTORM, 10},
};

static const WeatherTransition sTrans_Rainy[] = {
    {WEATHER_OVERWORLD_RAINY, 35},
    {WEATHER_OVERWORLD_THUNDERSTORM, 30},
    {WEATHER_OVERWORLD_CLOUDY, 25},
    {WEATHER_OVERWORLD_MISTY, 10},
};

static const WeatherTransition sTrans_Thunderstorm[] = {
    {WEATHER_OVERWORLD_THUNDERSTORM, 30},
    {WEATHER_OVERWORLD_STORMY, 40},
    {WEATHER_OVERWORLD_RAINY, 30},
};

static const WeatherTransition sTrans_Stormy[] = {
    {WEATHER_OVERWORLD_STORMY, 30},
    {WEATHER_OVERWORLD_RAINY, 45},
    {WEATHER_OVERWORLD_CLOUDY, 25},
};

static const WeatherTransition sTrans_Misty[] = {
    {WEATHER_OVERWORLD_MISTY, 40},
    {WEATHER_OVERWORLD_CLOUDY, 40},
    {WEATHER_OVERWORLD_SUNNY, 20},
};

static const WeatherTransition sTrans_Ash[] = {
    {WEATHER_OVERWORLD_ASH, 40},
    {WEATHER_OVERWORLD_SANDSTORM, 30},
    {WEATHER_OVERWORLD_DRY, 20},
    {WEATHER_OVERWORLD_CLOUDY, 10},
};

static const WeatherTransition sTrans_Sandstorm[] = {
    {WEATHER_OVERWORLD_SANDSTORM, 35},
    {WEATHER_OVERWORLD_DRY, 30},
    {WEATHER_OVERWORLD_ASH, 20},
    {WEATHER_OVERWORLD_CLOUDY, 15},
};

static const WeatherTransition sTrans_Dry[] = {
    {WEATHER_OVERWORLD_DRY, 35},
    {WEATHER_OVERWORLD_SUNNY, 35},
    {WEATHER_OVERWORLD_ASH, 15},
    {WEATHER_OVERWORLD_SANDSTORM, 15},
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

static u8 sCurrentOverworldWeather;
static s64 sWeatherStartTime;
static u32 sCurrentWeatherDuration;

static u32 RollWeatherDuration(u8 weather) {
  const WeatherStateData* data = &sWeatherStateData[weather];
  u32 range = data->maxDurationSec - data->minDurationSec;
  u32 result = data->minDurationSec + (
                 range ? Utils::GetRandomValue(range) : 0);
  return result >> 5;
}

static u8 PickNextWeather(u8 currentWeather) {
  const WeatherStateData* data = &sWeatherStateData[currentWeather];
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

  sCurrentOverworldWeather = WEATHER_OVERWORLD_SUNNY;
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
    u8 next = PickNextWeather(sCurrentOverworldWeather);

    sCurrentOverworldWeather = next;
    sWeatherStartTime = now;
    sCurrentWeatherDuration = RollWeatherDuration(next);

    overworld::WeatherManager::GetInstance().SetWeather(next);
  }

  overworld::WeatherManager::GetInstance().SetWeather(sCurrentOverworldWeather);
}
}