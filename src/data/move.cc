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

#include "data/move.h"

#include "menu/plugin_menu.h"

namespace data {

static const c8* CATEGORY_TYPES[] = {
    "Simple Damage",   "Simple Status",          "Simple Effect",
    "Simple Recover",  "Damage + Status",        "Effect + Status",
    "Damage + Effect", "Damage + Effect (User)", "Drain",
    "One-Hit KO",      "Field Effect",           "Side Effect"};

static const c8* DAMAGE_CATEGORIES[] = {"None", "Physical", "Special"};

static const c8* EFFECTS[] = {"None",
                              "Paralysis",
                              "Sleep",
                              "Ice",
                              "Burn",
                              "Poison",
                              "Confusion",
                              "Infatuation",
                              "Bind",
                              "Nightmare",
                              "Curse",
                              "Taunt",
                              "Encore",
                              "Rooted",
                              "Yawn",
                              "Perish Block",
                              "Ability Null",
                              "Scouted",
                              "Leech Seed",
                              "Throat Choke",
                              "Perish Song",
                              "Rooting",
                              "Trapped",
                              "Encore Repeat",
                              "Roost Effect",
                              "Move Lock",
                              "Charge Lock",
                              "Move Fixation",
                              "Always Hit",
                              "Always Hit Target",
                              "Levitate",
                              "Levitate Cancel",
                              "Telekinesis",
                              "Free Fall",
                              "Accuracy Up",
                              "Aqua Ring",
                              "Force Move Type",
                              "Smokescreen"};

static const c8* TURN_TYPES[] = {"None", "Permanent", "Turn Based",
                                 "While User Present", "Turn Or User Leave"};

static const c8* TARGETS[] = {
    "Other Select", "Friend User Select", "Friend Select", "Enemy Select",
    "Other All",    "Enemy All",          "Friend All",    "User",
    "All",          "Enemy Random",       "Field",         "Side Enemy",
    "Side Friend"};

static const c8* STATS[] = {"None",    "Attack", "Defense",  "Sp. Atk",
                            "Sp. Def", "Speed",  "Accuracy", "Evasion"};

void Move::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u16 move_id = 1;

  Move& data = GetInstance(move_id);

  menu.AddMove("Move", move_id).WithRefresh();

  menu.AddType("Type", data.type)

      .Add("Category", data.category)
      .WithArray(CATEGORY_TYPES, SIZE(CATEGORY_TYPES))

      .Add("Damage Cat.", data.damage_category)
      .WithArray(DAMAGE_CATEGORIES, SIZE(DAMAGE_CATEGORIES))

      .Add("Power", data.power)
      .Add("Accuracy", data.accuracy)
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

}  // namespace data