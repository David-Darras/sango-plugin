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

// The top page: one entry for each family below, then the quick access.
void LoadTopPage(MainApplication& app, void* args);

// The characters: the player and the people of the map, all the same.
void LoadCharactersPage(MainApplication& app, void* args);

// The cheats: features that are fun but do not change the data of the game.
void LoadCheatsPage(MainApplication& app, void* args);
void LoadOverworldCameraPage(MainApplication& app, void* args);

// The actions of the Overworld page: the applications (PC, Move Reminder...),
// the field moves and the location (with the teleport).
void LoadAppPage(MainApplication& app, void* args);
void LoadOverworldFieldMovePage(MainApplication& app, void* args);
void LoadLocationPage(MainApplication& app, void* args);

// Battles: the battle options of the plugin, then the current battle.
void LoadBattlePage(MainApplication& app, void* args);
void LoadBattleSettingsPage(MainApplication& app, void* args);
void LoadBattleSetupPage(MainApplication& app, void* args);
// ORAS only: starts a battle against any trainer.
void LoadTrainerBattlePage(MainApplication& app, void* args);
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
void LoadTallGrassPage(MainApplication& app, void* args);

// Pokémon: shiny Pokémon, randomizers and the rules for the Pokémon.
void LoadPokemonPage(MainApplication& app, void* args);
void LoadShinyPage(MainApplication& app, void* args);

// The game data: the tables of the species, the moves, the TMs and the types.
void LoadGameDataPage(MainApplication& app, void* args);
void LoadSpeciesDataPage(MainApplication& app, void* args);
void LoadMoveDataPage(MainApplication& app, void* args);
void LoadTechnicalMachinePage(MainApplication& app, void* args);
void LoadItemDataPage(MainApplication& app, void* args);

// The save data, one page for each part.
void LoadSaveDataPage(MainApplication& app, void* args);

// The graphics (lighting, filters, layouts and the model loader), the
// scripts, the title screen and the system.
void LoadGraphicsPage(MainApplication& app, void* args);
// The panes of the 2D layouts of the game (ORAS): the layouts of the screen,
// then one pane with its tree.
void LoadLayoutsPage(MainApplication& app, void* args);
// Hides the menu, the overlays and the 2D menus for a screenshot.
void LoadScreenshotPage(MainApplication& app, void* args);
void LoadPanePage(MainApplication& app, void* args);
// The scripts without code: the list, then the steps of one script.
void LoadDataScriptsPage(MainApplication& app, void* args);
void LoadDataScriptPage(MainApplication& app, void* args);
// The music, the weather and the time of the current map.
void LoadZoneSettingsPage(MainApplication& app, void* args);
// The map editor: opens the touch editor of the tiles and manages the changes.
void LoadMapEditorPage(MainApplication& app, void* args);
// The signs and the triggers of the map: the list, one sign, one trigger.
void LoadMapEventsPage(MainApplication& app, void* args);
void LoadSignEventPage(MainApplication& app, void* args);
void LoadTriggerEventPage(MainApplication& app, void* args);
// The texts of the game: the last texts that the game read, then one text.
void LoadTextEditorPage(MainApplication& app, void* args);
void LoadTextPage(MainApplication& app, void* args);
// The 3D models of the screen (ORAS): the list, one model, one material.
void LoadModelViewerPage(MainApplication& app, void* args);
void LoadModelPage(MainApplication& app, void* args);
void LoadMaterialPage(MainApplication& app, void* args);
// The skeleton of a 3D model (ORAS): the bones, then one bone.
void LoadSkeletonPage(MainApplication& app, void* args);
void LoadBonePage(MainApplication& app, void* args);
void LoadModelLoaderPage(MainApplication& app, void* args);
void LoadScriptPage(MainApplication& app, void* args);
void LoadTitleScreenPage(MainApplication& app, void* args);
void LoadNewGamePage(MainApplication& app, void* args);
void LoadSystemPage(MainApplication& app, void* args);
/// The last messages of the log of the plugin.
void LoadLogPage(MainApplication& app, void* args);
void LoadSoundPage(MainApplication& app, void* args);
void LoadGameTimePage(MainApplication& app, void* args);

// The plugin.
void LoadThemePage(MainApplication& app, void* args);
} // namespace ui

/// Keeps the model of the model loader page near the player.
void UpdateFollowingPokemon();

/// Removes the model from the scene before a battle starts. Call it before the
/// game destroys the scene.
void RemoveFollowingPokemonBeforeBattle();

/// The switch of the Pokémon that follows the player (page Cheats). The
/// Pokémon appears when the switch is on and the game is ready for it.
bool& FollowingPokemonSwitch();

/// The callback of the switch: removes the Pokémon when the switch goes off.
void ApplyFollowingPokemon(void*);
