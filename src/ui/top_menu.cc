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

#include "battle/manager.h"
#include "core/engine.h"
#include "core/pss_manager.h"
#include "menu/plugin_menu.h"
#include "menu/theme.h"
#include "overworld/overworld.h"
#include "savedata/savedata.h"
#include "system/sound.h"
#include "ui/game_time_menu.h"
#include "ui/global_data_menu.h"
#include "ui/renderer_menu.h"

namespace ui {
void LoadTopMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Renderer", LoadRendererMenu)
      .Add("Global Data", LoadGlobalDataMenu)
      .Add("Game Time", LoadGameTimeMenu)
      .Add("Save Data", savedata::SaveData::LoadMenu)
      .Add("Overworld", overworld::OverworldMenu)
      .Add("Battle", battle::Manager::LoadMenu)
      .Add("Game Speed", core::Engine::GetInstance().GetGameSpeed())
      .Add("PSS", PssManager::LoadMenu)
      .Add("Sound", Sound::LoadMenu)
      .Add("Plugin Theme", menu::Theme::LoadMenu);
}
} // namespace ui