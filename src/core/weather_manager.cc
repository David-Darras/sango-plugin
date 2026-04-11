// /*
//  * Copyright (C) 2026  David Darras
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  *

#include "overworld/weather_manager.h"

#include "menu/plugin_menu.h"

namespace overworld {

void WeatherManager::LoadMenu(menu::PluginMenu& menu, void* args) {
  static const c8* weathers[] = {"Sunny",  "Rainy",  "Thunderstorm",
                                 "Misty",  "Ash",    "Sandstorm",
                                 "Cloudy", "Stormy", "Dry"};

  WeatherManager& data = GetInstance();
  menu.Add("Requested Weather", data.GetRequestedWeather())
      .WithArray(weathers, SIZE(weathers))
      .Add("Current Weather", data.GetCurrentWeather())
      .WithArray(weathers, SIZE(weathers));
}

}  // namespace overworld
