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

#include "data/pokemon.h"

#include "menu/plugin_menu.h"

namespace data {

void Pokemon::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u16 species = 1;

  Pokemon& data = GetInstance(species);

  menu.AddSpecies("Species", species).WithBounds(0, 721).WithRefresh();

  menu.Add("Base HP", data.base_hp)
      .Add("Base Attack", data.base_attack)
      .Add("Base Defense", data.base_defense)
      .Add("Base Speed", data.base_speed)
      .Add("Base Sp. Atk", data.base_special_attack)
      .Add("Base Sp. Def", data.base_special_defense);

  menu.Add("EV Yield HP", &data.give_effort_values, 0, 2)
      .Add("EV Yield Attack", &data.give_effort_values, 2, 2)
      .Add("EV Yield Defense", &data.give_effort_values, 4, 2)
      .Add("EV Yield Speed", &data.give_effort_values, 6, 2)
      .Add("EV Yield SpAtk", &data.give_effort_values, 8, 2)
      .Add("EV Yield SpDef", &data.give_effort_values, 10, 2);

  menu.AddType("Type 1", data.type[0])
      .AddType("Type 2", data.type[1])
      .Add("Capture Rate", data.capture_rate)
      .Add("Base Friendship", data.base_friendship);

  menu.Add("Held Item 1", data.give_item[0])
      .Add("Held Item 2", data.give_item[1])
      .Add("Held Item 3", data.give_item[2]);

  menu.Add("Exp Yield", data.give_experience)
      .Add("Height", data.height)
      .Add("Weight", data.weight)
      .AddAbility("Ability 1", data.ability[0])
      .AddAbility("Ability 2", data.ability[1])
      .AddAbility("Ability 3 (Hidden)", data.ability[2])
      .Add("Gender", data.gender);

  menu.Add("Form Count", data.form_count)
      .Add("Egg Hatch Steps", data.egg_hatch_steps);
}

}  // namespace data