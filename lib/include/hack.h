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
 * @file hack.h
 * @brief The functions that a ROM hack writes. The library calls them.
 *
 * The library has a default entry point. It starts the plugin in this order:
 * 1. It installs the features of the library (plugin::InitializeEngine()).
 * 2. It calls SetUpHack(): your settings, moves, scripts...
 * 3. It opens the menu with LoadHackMenu() as the first page.
 * 4. At each frame, it calls UpdateHack(), then it updates and draws the menu.
 *
 * Write only the functions that you need. All of them are optional:
 * - without SetUpHack(), the game does not change,
 * - without LoadHackMenu(), the menu shows all the pages of the library,
 * - without UpdateHack(), nothing more happens at each frame.
 *
 * @code
 * #include "sango.h"
 *
 * void SetUpHack() {
 *   pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
 * }
 *
 * void LoadHackMenu(ui::MainApplication& app, void* args) {
 *   app.Add("Battle", ui::LoadBattlePage);
 * }
 * @endcode
 *
 * A product that must control the start of the plugin writes its own
 * Initialize() function (see plugin.h). The library then ignores these three
 * functions.
 *
 * @see docs/tutorials/01-create-your-rom-hack.md
 */

#pragma once

#include "common.h"

/// Sets the features of the ROM hack. The library calls it one time, when the
/// game starts, after it installs its features.
void SetUpHack();

/// Adds the entries of the first page of the menu.
void LoadHackMenu(ui::MainApplication& app, void* args);

/// Does the work of the ROM hack at each frame. The library calls it before
/// it updates the menu.
void UpdateHack();
