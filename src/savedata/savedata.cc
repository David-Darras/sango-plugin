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
#include "savedata/battle_box.h"
#include "savedata/box_manager.h"
#include "savedata/item_manager.h"
#include "savedata/misc.h"
#include "savedata/play_time.h"
#include "savedata/pokemon_amie.h"
#include "savedata/pokemon_box.h"
#include "savedata/pokemon_team.h"
#include "savedata/settings.h"
#include "savedata/trainer_status.h"

namespace savedata {

void SaveData::LoadMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Settings", Settings::LoadMenu)
      .Add("Play Time", PlayTime::LoadMenu)
      .Add("Team", PokemonTeam::LoadMenu)
      .Add("Battle Box", BattleBox::LoadMenu)
      .Add("PC", PokemonBox::LoadMenu)
      .Add("Trainer Status", TrainerStatus::LoadMenu)
      .Add("Items", ItemManager::LoadMenu)
      .Add("Pokemon-Amie", PokemonAmie::LoadMenu)
      .Add("Bag Metadata", BagManager::LoadMenu)
      .Add("Boxes Metadata", BoxManager::LoadMenu)
      .Add("Miscellaneous", Misc::LoadMenu);
}

}  // namespace savedata