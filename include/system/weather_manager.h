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

#ifndef SANGO_PLUGIN_WEATHER_MANAGER_H
#define SANGO_PLUGIN_WEATHER_MANAGER_H
#include "game_manager.h"

class WeatherManager {
 public:
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static WeatherManager& GetInstance() {
    return GameManager::GetInstance().GetWeatherManager();
  }

  u8& GetCurrentWeather() { return *(u8*)((uptr)this + 0x1C); }

  u8& GetRequestedWeather() { return *(u8*)((uptr)this + 0x1E); }
};

#endif  // SANGO_PLUGIN_WEATHER_MANAGER_H
