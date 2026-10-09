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
 * @file page_top.cc
 * @brief The root page of the overlay.
 */

#include "battle/patch/game_extension.h"
#include "core/native/event_manager.h"
#include "core/patch/game_speed.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/savedata.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
// The first page. The quick access comes first: the pinned entries and the
// recent entries. Then the pages, grouped by what the player wants to do.
// The pages for the creators of ROM hacks and the expert pages come last.
void LoadTopPage(MainApplication& app, void* args) {
  app.AddQuickAccess()
     .AddSection("Play")
     .Add("Game Speed", core::GameSpeed::GetInstance().game_speed)
     .WithBounds(-5, 5)
     .WithDescription("1 is the normal speed. 2 to 5 make the game faster. "
                      "-2 to -5 make it slower.")
     .Add("Player", LoadPlayerPage)
     .WithDescription("Field moves, apps, camera and look of the player.")
     .Add("Pokemon", LoadPokemonPage)
     .WithDescription("Shiny Pokemon, randomizers, species, moves and "
                      "TMs.")
     .Add("Battle", LoadBattlePage)
     .WithDescription("Battle options, and the Pokemon of the current "
                      "battle.")
     .Add("Overworld", LoadOverworldPage)
     .WithDescription("Weather, time of day, wild Pokemon and the map.")
     .AddSection("Create")
     .Add("Scripts", LoadScriptPage)
     .WithDescription("The overworld scripts and the C++ scripts.")
     .Add("Renderer", LoadRendererPage)
     .WithDescription("Lighting, outlines and color filters.")
     .Add("Title Screen", LoadTitleScreenPage)
     .WithDescription("The videos, the cry and the timing of the title "
                      "screen.")
     .AddSection("Advanced")
     .Add("Save Data", LoadSaveDataPage)
     .WithDescription("The raw data of the save file. Change it with "
                      "care.")
     .Add("System", LoadSystemPage)
     .WithDescription("Sounds and the clock of the game.")
     .Add("Plugin Theme", LoadThemePage)
     .WithDescription("The colors, the sounds and the button of the "
                      "menu.");
}
} // namespace ui