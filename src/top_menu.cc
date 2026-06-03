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
#include "core/pokemon_model.h"
#include "core/pss_manager.h"
#include "data/global_data.h"
#include "layout/layout.h"
#include "menu/plugin_menu.h"
#include "menu/theme.h"
#include "overworld/day_care.h"
#include "overworld/overworld.h"
#include "overworld/renderer.h"
#include "savedata/savedata.h"
#include "system/sound.h"

void TopMenu(menu::PluginMenu& menu, void* args) {
  overworld::MapManager& man = overworld::MapManager::GetInstance();

  menu.Add("Game Speed", core::Engine::GetInstance().GetGameSpeed())
      .Add("Pokemon Model", PokemonModelMenu)
      .Add("Global Data", global_data::GlobalDataMenu)
      .Add("Save Data", savedata::SaveData::LoadMenu)
      .Add("Overworld", overworld::OverworldMenu)
      .Add("Layout", layout::LayoutMenu)
      .Add("Battle", battle::Manager::LoadMenu)
      .Add("PSS", PssManager::LoadMenu)
      .Add("Day Care", overworld::DayCareMenu)
      .Add("Sound", Sound::LoadMenu)
      .Add("Plugin Theme", menu::Theme::LoadMenu);
}