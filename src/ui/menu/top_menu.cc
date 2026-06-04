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

#include "feature/feature_engine.h"
#include "ui/plugin_menu.h"
#include "game/overworld/weather_manager.h"
#include "ui/top_menu.h"

namespace ui {
void LoadTopMenu(PluginMenu& menu, void* args) {
  static const c8* WEATHERS[] = {
      "Sunny", "Rainy", "Thunderstorm",
      "Misty", "Ash", "Sandstorm",
      "Cloudy", "Stormy", "Dry"
  };

  auto& weather_manager = overworld::WeatherManager::GetInstance();

  menu.Add("Battle Config", LoadBattleConfigMenu);

  menu.Add("Game Speed", feature::EngineHookContext::GetInstance().game_speed)
      .Add("Weather", weather_manager.GetRequestedWeather())
      .WithArray(WEATHERS, SIZE(WEATHERS))
      .AddSeparator();

  menu.Add("Renderer", LoadRendererMenu)
      .Add("Global Data", LoadGlobalDataMenu)
      .Add("Game Time", LoadGameTimeMenu)
      .Add("Save Data", LoadSaveDataMenu)
      .Add("Overworld", LoadOverworldMenu)
      .Add("Day Care", LoadDayCareMenu)
      .Add("Battle", LoadBattleMenu)
      .Add("Sound", LoadSoundMenu)
      .Add("Plugin Theme", LoadThemeMenu);
}
} // namespace ui