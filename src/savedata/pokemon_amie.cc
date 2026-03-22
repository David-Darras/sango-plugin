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

#include "savedata/pokemon_amie.h"

#include "menu/plugin_menu.h"

namespace savedata {

void PokemonAmie::LoadMenu(menu::PluginMenu& menu) {
  static u8 puff_idx = 0;
  PokemonAmie& data = GetInstance();

  menu.Add("Select Slot", puff_idx)
      .WithBounds(0, kMaxPokePuffs - 1)
      .WithRefresh();

  menu.Add("Poké Puff ID", data.poke_puffs[puff_idx])
      .WithBounds(0, kMaxPokePuffId);

  menu.Add("Last Opened (Days)", data.last_opened_timestamp);
}

}  // namespace savedata