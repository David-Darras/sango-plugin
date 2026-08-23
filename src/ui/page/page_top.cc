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
#include "game/overworld/weather_manager.h"
#include "ui/main_application.h"
#include "ui/page/page_top.h"

#include "config_manager.h"

namespace ui {
void LoadTopPage(MainApplication& app, void* args) {
  static const c8* WEATHERS[] = {
      "Sunny", "Rainy", "Thunderstorm",
      "Misty", "Ash", "Sandstorm",
      "Cloudy", "Stormy", "Dry"
  };

  auto& weather_manager = overworld::WeatherManager::GetInstance();

  app.Add("Game Speed", feature::Engine::GetInstance().game_speed)
     .Add("Overworld", LoadOverworldPage)
     .Add("Battle", LoadBattlePage)
     .Add("Save Data", LoadSaveDataPage)
     .Add("Global Data", LoadGlobalDataPage)
     .Add("Renderer", LoadRendererPage)
     .Add("App", LoadAppPage)
     .Add("Battle Config", LoadBattleConfigPage)
     .Add("Game Time", LoadGameTimePage)
     .Add("Weather", weather_manager.GetRequestedWeather())
     .WithArray(WEATHERS, SIZE(WEATHERS))
     .Add("Day Care", LoadDayCarePage)
     .Add("Sound", LoadSoundPage)
     .Add("Scripts", LoadScriptPage)
     .Add("Plugin Theme", LoadThemePage)
     .Add("Save Config", ConfigManager::Save);
}
} // namespace ui