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
 * @file pages.h
 * @brief The menu pages of the library.
 *
 * The overlay adds all the pages under LoadTopPage. A product adds the pages
 * that it needs to its own pages: app.Add("Battle", ui::LoadBattlePage).
 */

#pragma once
#include "common.h"

namespace ui {
class MainApplication;

// The top page: the quick settings and one entry for each family below.
void LoadTopPage(MainApplication& app, void* args);

// The player: the actions and the appearance of the player in the overworld.
void LoadPlayerPage(MainApplication& app, void* args);
void LoadOverworldFieldMovePage(MainApplication& app, void* args);
void LoadAppPage(MainApplication& app, void* args);
void LoadOverworldCameraPage(MainApplication& app, void* args);
void LoadPlayerModelPage(MainApplication& app, void* args);

// Battles: the battle options of the plugin, then the current battle.
void LoadBattlePage(MainApplication& app, void* args);
void LoadBattleSettingsPage(MainApplication& app, void* args);
void LoadBattleSetupPage(MainApplication& app, void* args);
void LoadTypeChartPage(MainApplication& app, void* args);
// ORAS only: the table of the static encounters (legendary Pokémon...).
void LoadStaticEncounterPage(MainApplication& app, void* args);
void LoadBattleLivePage(MainApplication& app, void* args);

// The overworld: the current map and its data.
void LoadOverworldPage(MainApplication& app, void* args);
void LoadWeatherPage(MainApplication& app, void* args);
void LoadTimeOfDayPage(MainApplication& app, void* args);
void LoadWorldLayoutPage(MainApplication& app, void* args);
void LoadOverworldMapTilePage(MainApplication& app, void* args);
void LoadPropModelPage(MainApplication& app, void* args);
void LoadOverworldEncounterPage(MainApplication& app, void* args);
void LoadDayCarePage(MainApplication& app, void* args);

// Pokémon: shiny Pokémon, randomizers, and the species and move tables.
void LoadPokemonPage(MainApplication& app, void* args);
void LoadShinyPage(MainApplication& app, void* args);
void LoadSpeciesDataPage(MainApplication& app, void* args);
void LoadMoveDataPage(MainApplication& app, void* args);
void LoadTechnicalMachinePage(MainApplication& app, void* args);
void LoadModelLoaderPage(MainApplication& app, void* args);

// The save data, one page for each part.
void LoadSaveDataPage(MainApplication& app, void* args);

// The renderer, the scripts, the title screen and the system.
void LoadRendererPage(MainApplication& app, void* args);
void LoadScriptPage(MainApplication& app, void* args);
void LoadTitleScreenPage(MainApplication& app, void* args);
void LoadNewGamePage(MainApplication& app, void* args);
void LoadSystemPage(MainApplication& app, void* args);
/// The last messages of the log of the plugin.
void LoadLogPage(MainApplication& app, void* args);
/// The event flags of the save data, 16 at a time, with their names.
void LoadEventFlagsPage(MainApplication& app, void* args);
/// The script variables of the save data, 16 at a time, with their names.
void LoadEventVariablesPage(MainApplication& app, void* args);
void LoadSoundPage(MainApplication& app, void* args);
void LoadGameTimePage(MainApplication& app, void* args);

// The plugin.
void LoadThemePage(MainApplication& app, void* args);
} // namespace ui

/// Keeps the model of the model loader page near the player.
void UpdateFollowingPokemon();
