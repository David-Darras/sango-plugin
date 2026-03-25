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

#include "savedata/pokemon_team.h"

#include "menu/plugin_menu.h"

namespace savedata {

static u8 slot_idx = 0;

void PokemonTeam::LoadMenu(menu::PluginMenu& menu, void* args) {
  PokemonTeam& data = GetInstance();

  menu.Add("Count", data.count)
      .Add("Slot Index", slot_idx)
      .WithBounds(0, kMaxSlots - 1)
      .WithRefresh()
      .Add("Edit Pokemon", PokemonCoreData::LoadMenu,
           data.pokemons[slot_idx]->core);
}

}  // namespace savedata