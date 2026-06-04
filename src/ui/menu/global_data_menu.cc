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

#include "game/game_time_manager.h"
#include "global_data/move.h"
#include "global_data/pokemon.h"
#include "global_data/move.inc"
#include "ui/plugin_menu.h"

namespace ui {
void LoadMoveMenu(PluginMenu& menu, void* args) {
  static u16 move = 0;
  auto& data = global_data::Move::GetInstance(move);

  menu.AddMove("Move", move).WithRefresh();

  menu.AddType("Type", data.type)

      .Add("Category", data.category)
      .WithArray(CATEGORY_TYPES, SIZE(CATEGORY_TYPES))

      .Add("Damage Cat.", data.damage_category)
      .WithArray(DAMAGE_CATEGORIES, SIZE(DAMAGE_CATEGORIES))

      .Add("Power", data.power)
      .Add("Accuracy (%)", data.accuracy)
      .WithBounds(0, 100)
      .Add("Base PP", data.base_pp)
      .Add("Priority", data.priority)
      .Add("Min Hit Count", &data.hit_count, 0, 4)
      .Add("Max Hit Count", &data.hit_count, 4, 4);

  menu.Add("Effect", data.effect_id)
      .WithArray(EFFECTS, SIZE(EFFECTS))

      .Add("Effect Rate", data.effect_rate)
      .Add("Effect Turn Type", data.effect_turn_type)
      .WithArray(TURN_TYPES, SIZE(TURN_TYPES))

      .Add("Effect Min Turns", data.min_turns)
      .Add("Effect Max Turns", data.max_turns)
      .Add("Crit Stage", data.crit_stage)
      .Add("Flinch Rate", data.flinch_rate);

  menu.Add("Recoil", data.recoil)
      .Add("Drain", data.drain)
      .Add("Target", data.target)
      .WithArray(TARGETS, SIZE(TARGETS));

  menu.Add("Stat 1", data.stat_id[0])
      .WithArray(STATS, SIZE(STATS))
      .Add("Stat Stage 1", data.stat_stages[0])
      .Add("Stat Rate 1", data.stat_rate[0])
      .Add("Stat 2", data.stat_id[1])
      .WithArray(STATS, SIZE(STATS))
      .Add("Stat Stage 2", data.stat_stages[1])
      .Add("Stat Rate 2", data.stat_rate[1])
      .Add("Stat 3", data.stat_id[2])
      .WithArray(STATS, SIZE(STATS))
      .Add("Stat Stage 3", data.stat_stages[2])
      .Add("Stat Rate 3", data.stat_rate[2]);

  menu.Add("Flags", data.flags);
}

void LoadPokemonMenu(PluginMenu& menu, void* args) {
  static u16 species = 0;
  auto& data = global_data::Pokemon::GetInstance(species);

  menu.AddSpecies("Species", species).WithRefresh();

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
      .Add("Capture Rate (%)", data.capture_rate)
      .WithBounds(0, 100)
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

void LoadGlobalDataMenu(PluginMenu& menu, void* args) {
  menu.Add("Global Pokemon Data", LoadPokemonMenu)
      .Add("Global Move Data", LoadMoveMenu);
}
} // namespace global_data