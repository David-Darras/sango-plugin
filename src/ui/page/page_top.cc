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

#include "feature/core/feature_engine.h"
#include "game/overworld/weather_manager.h"
#include "ui/main_application.h"
#include "ui/page/page_top.h"

#include "config_manager.h"
#include "feature/pokemon/feature_shiny.h"

namespace ui {
void LoadShinyPage(MainApplication& app, void* args) {
  static const c8* SHINY_RATES[] = {"Off", "1/1", "1/2", "1/4", "1/8", "1/16",
                                    "1/32", "1/64", "1/128", "1/256", "1/512",
                                    "1/1024", "1/2048", "1/4096", "1/8192",
                                    "1/16384", "1/32768", "1/65536", "1/131072",
                                    "1/262144", "1/524288", "1/1048576"};

  auto& shiny = feature::Shiny::GetInstance();

  app.Add("Shiny Rate", shiny.rate)
     .WithArray(SHINY_RATES, SIZE(SHINY_RATES))
     .WithBounds(0, SIZE(SHINY_RATES) - 1);
}

void LoadTopPage(MainApplication& app, void* args) {
  app.Add("Game Speed", feature::Engine::GetInstance().game_speed)
     .Add("Repel", CheatCodeId::kNoEncounter)
     .Add("Overworld", LoadOverworldPage)
     .Add("Save Data", LoadSaveDataPage)
     .Add("Global Data", LoadGlobalDataPage)
     .Add("Renderer", LoadRendererPage)
     .Add("Battle", LoadBattlePage)
     .Add("Battle Config", LoadBattleConfigPage);

  LoadShinyPage(app, args);

  app.Add("Sound", LoadSoundPage)
     .Add("Game Time", LoadGameTimePage)
     .Add("Title Screen", LoadTitleScreenPage)
     .Add("Scripts", LoadScriptPage)
     .Add("Plugin Theme", LoadThemePage)
     .Add("Save Config", ConfigManager::Save);
}
} // namespace ui