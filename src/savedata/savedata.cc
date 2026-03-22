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
#include "savedata/savedata.h"

#include "menu/plugin_menu.h"
#include "savedata/bag_manager.h"
#include "savedata/box_manager.h"
#include "savedata/item_manager.h"
#include "savedata/misc.h"
#include "savedata/pokemon_amie.h"

namespace savedata {

void SaveData::LoadMenu(menu::PluginMenu& menu) {
  menu.Add("Items", ItemManager::LoadMenu)
      .Add("Pokemon-Amie", PokemonAmie::LoadMenu)
      .Add("Bag", BagManager::LoadMenu)
      .Add("Boxes", BoxManager::LoadMenu)
      .Add("Miscellaneous", Misc::LoadMenu);
}

}  // namespace savedata