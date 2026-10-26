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
namespace {
const c8* kFrameRateNames[] = {"Default", "30 FPS", "60 FPS"};
} // namespace

// The first page. The entries are in groups that tell what they are for: the
// speed of the game, what the player plays with, the data, the look and the
// sound, then the plugin. The quick access (the pinned entries) comes last.
void LoadTopPage(MainApplication& app, void* args) {
  app.AddSection("Speed")
     .Add("Game Speed", core::GameSpeed::GetInstance().game_speed)
     .WithBounds(-5, 5)
     .WithDescription("1 is the normal speed. 2 to 5 make the game faster. "
         "-2 to -5 make it slower.")
     .Add("Frame Rate", core::GameSpeed::GetInstance().frame_rate)
     .WithArray(kFrameRateNames, SIZE(kFrameRateNames))
     .WithDescription("Game: the engine chooses (30 frames in the field). "
         "60 FPS is smoother, but some animations and scripts run too fast.")
     .AddSection("Play")
     .Add("Overworld", LoadOverworldPage)
     .WithDescription("Applications, field moves, location, weather, wild "
         "Pokemon, characters, camera and objects of the map.")
     .Add("Battle", LoadBattlePage)
     .WithDescription("Battle options, new battles, and the Pokemon of the "
         "current battle.")
     .Add("Pokemon", LoadPokemonPage)
     .WithDescription("Shiny Pokemon, randomizers and rules for the "
         "Pokemon.")
     .Add("Cheats", LoadCheatsPage)
     .WithDescription("Noclip, Auto Surf, Swarm, Day Care and other fun "
         "features.")
     .AddSection("Data")
     .Add("Save Data", LoadSaveDataPage)
     .WithDescription("The save file: money, badges, bag, team, boxes... "
         "Change it with care.")
     .Add("Game Data", LoadGameDataPage)
     .WithDescription("The tables of the game: species, moves, TMs, items "
         "and the type chart.")
     .Add("Scripts", LoadScriptPage)
     .WithDescription("The overworld scripts and the C++ scripts.")
     .AddSection("Look And Sound")
     .Add("Graphics", LoadGraphicsPage)
     .WithDescription("Lighting, outlines, color filters, layouts and the "
         "model loader.")
     .Add("Sound", LoadSoundPage)
     .WithDescription("Plays the music, the sound effects and the cries.")
     .Add("Title Screen", LoadTitleScreenPage)
     .WithDescription("The videos, the cry and the timing of the title "
         "screen, and the start of a new game.")
     .AddSection("Plugin")
     .Add("Plugin Theme", LoadThemePage)
     .WithDescription("The colors, the sounds and the button of the "
         "menu.")
     .Add("Log", LoadLogPage)
     .WithDescription("The last messages of the plugin.")
     .Add("System", LoadSystemPage)
     .WithDescription("The performance overlay and the clock of the "
         "game.")
     .AddQuickAccess();
}
} // namespace ui
