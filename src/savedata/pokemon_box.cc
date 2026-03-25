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

#include "savedata/pokemon_box.h"

#include "menu/plugin_menu.h"

namespace savedata {

static struct PokemonBoxContext {
  u8 box_idx;
  u8 slot_idx;
} ctx = {0, 0};

void PokemonBox::LoadMenu(menu::PluginMenu& menu, void* args) {
  PokemonBox& data = GetInstance();
  menu.Add("Box Index", ctx.box_idx)
      .WithRefresh()
      .Add("Slot Index", ctx.slot_idx)
      .WithRefresh()
      .Add("Edit Pokemon", PokemonCoreData::LoadMenu,
           &data.boxes[ctx.box_idx].pokemons[ctx.slot_idx]);
}

}  // namespace savedata
