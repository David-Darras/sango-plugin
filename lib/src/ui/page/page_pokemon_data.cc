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

/**
 * @file page_pokemon_data.cc
 * @brief The menu pages of the species data, the move data and the TMs.
 */

#include "pokemon/native/move_data.h"
#include "pokemon/native/species_data.h"
#include "pokemon/native/technical_machine_table.h"
#include "pokemon/data/move.inc"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
// The TMs of the table: TM01 to TM100. The menu does not show the HMs.
constexpr u32 kTechnicalMachineCount = 100;
} // namespace

void LoadMoveDataPage(MainApplication& app, void* args) {
  static MoveId move = MoveId::kNone;
  auto& data = pokemon::MoveData::GetInstance(move);

  app.AddMove("Move", move).WithRefresh();

  app.AddSection("Move")
     .AddType("Type", data.type)
     .Add("Category", data.category)
     .WithArray(CATEGORY_TYPES, SIZE(CATEGORY_TYPES))
     .Add("Damage Category", data.damage_category)
     .WithArray(DAMAGE_CATEGORIES, SIZE(DAMAGE_CATEGORIES))
     .Add("Power", data.power)
     .Add("Accuracy", data.accuracy)
     .WithBounds(0, 101)
     .WithDescription("In percent. 101: the move never misses.")
     .Add("Base PP", data.base_pp)
     .Add("Priority", data.priority)
     .WithMin(-7)
     .WithMax(5)
     .Add("Min Hit Count", &data.hit_count, 0, 4)
     .Add("Max Hit Count", &data.hit_count, 4, 4)
     .Add("Target", data.target)
     .WithArray(TARGETS, SIZE(TARGETS));

  app.AddSection("Effect")
     .Add("Status Condition", data.effect_id)
     .WithArray(EFFECTS, SIZE(EFFECTS))
     .Add("Chance (%)", data.effect_rate)
     .WithBounds(0, 100)
     .Add("Duration", data.effect_turn_type)
     .WithArray(TURN_TYPES, SIZE(TURN_TYPES))
     .Add("Min Turns", data.min_turns)
     .Add("Max Turns", data.max_turns)
     .Add("Critical Hit Stage", data.crit_stage)
     .Add("Flinch Chance (%)", data.flinch_rate)
     .WithBounds(0, 100)
     .Add("Recoil (%)", data.recoil)
     .WithDescription("The damage to the user, in percent of the damage. "
                      "The value is negative.")
     .Add("Drain (%)", data.drain)
     .WithDescription("The HP that the user gets back, in percent of the "
                      "damage.");

  app.AddSection("Stat Changes")
     .Add("Stat 1", data.stat_id[0])
     .WithArray(STATS, SIZE(STATS))
     .Add("Stages 1", data.stat_stages[0])
     .WithMin(-6)
     .WithMax(6)
     .Add("Chance 1 (%)", data.stat_rate[0])
     .WithBounds(0, 100)
     .Add("Stat 2", data.stat_id[1])
     .WithArray(STATS, SIZE(STATS))
     .Add("Stages 2", data.stat_stages[1])
     .WithMin(-6)
     .WithMax(6)
     .Add("Chance 2 (%)", data.stat_rate[1])
     .WithBounds(0, 100)
     .Add("Stat 3", data.stat_id[2])
     .WithArray(STATS, SIZE(STATS))
     .Add("Stages 3", data.stat_stages[2])
     .WithMin(-6)
     .WithMax(6)
     .Add("Chance 3 (%)", data.stat_rate[2])
     .WithBounds(0, 100);

  app.AddSection("Other")
     .Add("Flags", data.flags)
     .WithDescription("One bit for each flag: contact, sound, punch...");
}

void LoadSpeciesDataPage(MainApplication& app, void* args) {
  static SpeciesId species = SpeciesId::kNone;
  static FormId form = FormId::kNormal;
  static u8 tm_index = 0;
  auto& data = pokemon::SpeciesData::GetInstance(species, form);

  app.AddSpecies("Species", species).WithRefresh()
     .Add("Form", form).WithRefresh();

  app.AddSection("Base Stats")
     .Add("HP", data.base_hp)
     .Add("Attack", data.base_attack)
     .Add("Defense", data.base_defense)
     .Add("Sp. Atk", data.base_special_attack)
     .Add("Sp. Def", data.base_special_defense)
     .Add("Speed", data.base_speed);

  app.AddSection("Types and Abilities")
     .AddType("Type 1", data.type[0])
     .AddType("Type 2", data.type[1])
     .AddAbility("Ability 1", data.ability[0])
     .AddAbility("Ability 2", data.ability[1])
     .AddAbility("Hidden Ability", data.ability[2]);

  app.AddSection("Wild Pokemon")
     .Add("Catch Rate", data.capture_rate)
     .WithDescription("3: hard to catch (legendary Pokemon). 255: easy to "
                      "catch.")
     .AddItem("Held Item 1", data.give_item[0])
     .AddItem("Held Item 2", data.give_item[1])
     .AddItem("Held Item 3", data.give_item[2])
     .Add("Gender Ratio", data.gender)
     .WithDescription("0: male only. 254: female only. 255: no gender. "
                      "127: half male, half female.")
     .Add("Escape Rate", data.escape_rate);

  app.AddSection("Rewards")
     .Add("Exp. Yield", data.give_experience)
     .Add("EV Yield HP", &data.give_effort_values, 0, 2)
     .Add("EV Yield Attack", &data.give_effort_values, 2, 2)
     .Add("EV Yield Defense", &data.give_effort_values, 4, 2)
     .Add("EV Yield Speed", &data.give_effort_values, 6, 2)
     .Add("EV Yield Sp. Atk", &data.give_effort_values, 8, 2)
     .Add("EV Yield Sp. Def", &data.give_effort_values, 10, 2);

  app.AddSection("Breeding and Size")
     .Add("Base Friendship", data.base_friendship)
     .Add("Egg Cycles", data.egg_hatch_steps)
     .WithDescription("The number of step cycles to hatch an Egg.")
     .Add("Height", data.height)
     .WithDescription("In tenths of a meter: 7 is 0.7 m.")
     .Add("Weight", data.weight)
     .WithDescription("In tenths of a kilogram: 69 is 6.9 kg.")
     .Add("Form Count", data.form_count)
     .WithReadOnly();

  u32* tm_bits = &data.technical_moves[tm_index / 32];
  app.AddSection("TMs")
     .Add("TM", tm_index)
     .WithArray(GetNumberedNames("TM%02u", kTechnicalMachineCount),
                kTechnicalMachineCount)
     .WithRefresh()
     .AddMove("Move", pokemon::TechnicalMachineTable::GetTable()[tm_index])
     .WithReadOnly()
     .Add("Can Learn It", tm_bits, tm_index % 32, 1);
}

void LoadTechnicalMachinePage(MainApplication& app, void* args) {
  static u8 tm_index = 0;
  MoveId* table = pokemon::TechnicalMachineTable::GetTable();

  app.Add("TM", tm_index)
     .WithArray(GetNumberedNames("TM%02u", kTechnicalMachineCount),
                kTechnicalMachineCount)
     .WithRefresh()
     .AddMove("Move", table[tm_index])
     .WithDescription("The move that the TM teaches. Species Data shows "
                      "which species can learn it.");
}
} // namespace ui
