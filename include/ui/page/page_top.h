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

namespace ui {
class MainApplication;
void LoadKaizoPage(MainApplication& app, void* args);
void LoadThemePage(MainApplication& app, void* args);
void LoadBattleConfigPage(MainApplication& app, void* args);
void LoadBattlePage(MainApplication& app, void* args);
void LoadDayCarePage(MainApplication& app, void* args);
void LoadGameTimePage(MainApplication& app, void* args);
void LoadGlobalDataPage(MainApplication& app, void* args);
void LoadModelLoaderPage(MainApplication& app, void* args);
void LoadOverworldPage(MainApplication& app, void* args);
void LoadRendererPage(MainApplication& app, void* args);
void LoadSaveDataPage(MainApplication& app, void* args);
void LoadScriptPage(MainApplication& app, void* args);
void LoadSoundPage(MainApplication& app, void* args);
void LoadTopPage(MainApplication& app, void* args);
void LoadAppPage(MainApplication& app, void* args);
void LoadTitleScreenPage(MainApplication& app, void* args);
}