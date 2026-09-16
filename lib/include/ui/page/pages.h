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

#pragma once
#include "common.h"

/// The menu pages the library ships. The overlay assembles them all under
/// LoadTopPage; a product picks the ones it wants under its own root page.
namespace ui {
class MainApplication;

// Root: the quick toggles and one entry per family below.
void LoadTopPage(MainApplication& app, void* args);

// Player: what the player can do or become on the field.
void LoadPlayerPage(MainApplication& app, void* args);
void LoadOverworldFieldMovePage(MainApplication& app, void* args);
void LoadAppPage(MainApplication& app, void* args);
void LoadOverworldCameraPage(MainApplication& app, void* args);
void LoadPlayerModelPage(MainApplication& app, void* args);

// Battle: the plugin's battle options, then the live battle.
void LoadBattlePage(MainApplication& app, void* args);
void LoadBattleSettingsPage(MainApplication& app, void* args);
void LoadBattleSetupPage(MainApplication& app, void* args);
void LoadTypeChartPage(MainApplication& app, void* args);
void LoadBattleLivePage(MainApplication& app, void* args);

// Overworld: the map being played and its data.
void LoadOverworldPage(MainApplication& app, void* args);
void LoadWeatherPage(MainApplication& app, void* args);
void LoadWorldLayoutPage(MainApplication& app, void* args);
void LoadOverworldMapTilePage(MainApplication& app, void* args);
void LoadPropModelPage(MainApplication& app, void* args);
void LoadOverworldEncounterPage(MainApplication& app, void* args);
void LoadDayCarePage(MainApplication& app, void* args);

// Pokemon: shininess, randomizers and the game's species/move tables.
void LoadPokemonPage(MainApplication& app, void* args);
void LoadShinyPage(MainApplication& app, void* args);
void LoadSpeciesDataPage(MainApplication& app, void* args);
void LoadMoveDataPage(MainApplication& app, void* args);
void LoadModelLoaderPage(MainApplication& app, void* args);

// Save data, by segment.
void LoadSaveDataPage(MainApplication& app, void* args);

// Renderer, scripts, title screen, system.
void LoadRendererPage(MainApplication& app, void* args);
void LoadScriptPage(MainApplication& app, void* args);
void LoadTitleScreenPage(MainApplication& app, void* args);
void LoadNewGamePage(MainApplication& app, void* args);
void LoadSystemPage(MainApplication& app, void* args);
void LoadSoundPage(MainApplication& app, void* args);
void LoadGameTimePage(MainApplication& app, void* args);

// The plugin itself.
void LoadThemePage(MainApplication& app, void* args);
} // namespace ui

/// Keeps the model spawned by the model loader page next to the player.
void UpdateFollowingPokemon();
