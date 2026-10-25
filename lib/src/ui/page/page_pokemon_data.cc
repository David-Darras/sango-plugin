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

#include <cstddef>
#include <cstring>

#include "pokemon/native/item_data.h"
#include "pokemon/native/move_data.h"
#include "pokemon/patch/item_customizer.h"
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
     .WithIcons(IconKind::kMoveCategory)
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
     .WithIcons(IconKind::kStatus)
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

namespace {
using pokemon::ItemData;

// The item of the page, and a copy of its data that the entries change.
ItemId item_id = static_cast<ItemId>(1);
ItemId loaded_item_id = static_cast<ItemId>(0xFFFF);
alignas(4) u8 item_buffer[sizeof(ItemData) + 4];

static_assert(offsetof(ItemData, price) == 0, "ItemData layout");
static_assert(offsetof(ItemData, power) == 3, "ItemData layout");
static_assert(offsetof(ItemData, fling_power) == 6, "ItemData layout");
static_assert(offsetof(ItemData, overworld_function) == 10, "ItemData layout");
static_assert(offsetof(ItemData, sort_number) == 15, "ItemData layout");
static_assert(offsetof(ItemData, hp_ev_value) == 23, "ItemData layout");
static_assert(offsetof(ItemData, friendship_value_3) == 33,
              "ItemData layout");

// The word (4 bytes) that holds the byte `byte` of the data. The entries of
// bits read 32 bits at an aligned address.
u32* Word(ItemData& data, u32 byte) {
  return reinterpret_cast<u32*>(reinterpret_cast<u8*>(&data) + (byte & ~3u));
}

// Adds an entry for `width` bits of a byte of the data.
#define ITEM_BITS(name, byte, bit, width) \
  Add(name, Word(data, byte), ((byte) % 4) * 8 + (bit), width)

// Keeps the changes: the game reads this copy each time it reads the item.
// An item with the data of the game keeps nothing.
void KeepItemChanges(void*) {
  ItemData& data = *reinterpret_cast<ItemData*>(item_buffer);
  ItemData original(data.id);
  if (memcmp(&data, &original, offsetof(ItemData, _1)) == 0) {
    pokemon::ItemCustomizer::ClearOverride(data.id);
  } else if (!pokemon::ItemCustomizer::SetOverride(data)) {
    MainApplication::GetInstance().ShowToast(
        "16 items at most: reset an item first.");
  }
}

void ResetItem(void*) {
  pokemon::ItemCustomizer::ClearOverride(item_id);
  loaded_item_id = static_cast<ItemId>(0xFFFF);
  MainApplication::GetInstance().Refresh();
}
} // namespace

void LoadItemDataPage(MainApplication& app, void* args) {
  app.AddItem("Item", item_id).WithRefresh();

  // Load the data of the item: the changed copy, or the data of the game.
  if (item_id != loaded_item_id) {
    const ItemData* changed = pokemon::ItemCustomizer::GetOverride(item_id);
    if (changed != nullptr) {
      memcpy(item_buffer, changed, sizeof(ItemData));
    } else {
      ItemData fresh(item_id);
      memcpy(item_buffer, &fresh, sizeof(ItemData));
    }
    loaded_item_id = item_id;
  }
  ItemData& data = *reinterpret_cast<ItemData*>(item_buffer);

  app.OnChange(KeepItemChanges);
  app.AddSection("Item")
     .Add("Price", data.price)
     .WithDescription("The price in a Poke Mart. The shop sells it for this "
                      "price; the player sells it for half.")
     .Add("Sort Number", data.sort_number)
     .ITEM_BITS("Overworld Pocket", 8, 7, 4)
     .WithDescription("The pocket of the Bag outside of a battle.")
     .ITEM_BITS("Battle Pocket", 8, 11, 5)
     .ITEM_BITS("Key Item", 8, 5, 1)
     .ITEM_BITS("Can Be Registered", 8, 6, 1)
     .Add("Reset This Item", ResetItem)
     .WithDescription("Gives the original data of the item back.")

     .AddSection("Use")
     .Add("Use On A Pokemon", data.use_on_pokemon)
     .Add("Overworld Function", data.overworld_function)
     .WithDescription("The effect outside of a battle.")
     .Add("Battle Function", data.battle_function)
     .WithDescription("The effect in a battle.")
     .Add("Item Type", data.item_type)
     .ITEM_BITS("Consumed On Use", 14, 4, 4)
     .WithDescription("0: the item is consumed. Another value: it stays.")

     .AddSection("Held By A Pokemon")
     .Add("Hold Effect", data.hold_effect)
     .Add("Hold Effect Power", data.power)
     .ITEM_BITS("Consumed In Battle", 14, 0, 4)
     .Add("Fling Effect", data.fling_effect)
     .Add("Fling Power", data.fling_power)
     .Add("Pluck Effect", data.pluck_effect)
     .Add("Natural Gift Power", data.natural_gift_power)
     .ITEM_BITS("Natural Gift Type", 8, 0, 5)

     .AddSection("Healing")
     .ITEM_BITS("Cures Sleep", 16, 0, 1)
     .ITEM_BITS("Cures Poison", 16, 1, 1)
     .ITEM_BITS("Cures Burn", 16, 2, 1)
     .ITEM_BITS("Cures Freeze", 16, 3, 1)
     .ITEM_BITS("Cures Paralysis", 16, 4, 1)
     .ITEM_BITS("Cures Confusion", 16, 5, 1)
     .ITEM_BITS("Cures Infatuation", 16, 6, 1)
     .ITEM_BITS("Protects Stat Stages", 16, 7, 1)
     .ITEM_BITS("Restores HP", 21, 2, 1)
     .Add("HP Restored", data.hp_restore_value)
     .WithDescription("The HP that the item gives. 255: all the HP.")
     .ITEM_BITS("Restores PP", 21, 0, 1)
     .ITEM_BITS("Restores PP Of All Moves", 21, 1, 1)
     .Add("PP Restored", data.pp_restore_value)
     .ITEM_BITS("Raises Max PP (PP Up)", 20, 6, 1)
     .ITEM_BITS("Raises Max PP To The Top (PP Max)", 20, 7, 1)
     .ITEM_BITS("Revives", 17, 0, 1)
     .ITEM_BITS("Revives All", 17, 1, 1)
     .ITEM_BITS("Raises The Level", 17, 2, 1)
     .ITEM_BITS("Evolves", 17, 3, 1)

     .AddSection("Battle Boosts")
     .ITEM_BITS("Attack Stage", 17, 4, 4)
     .ITEM_BITS("Defense Stage", 18, 0, 4)
     .ITEM_BITS("Sp. Atk Stage", 18, 4, 4)
     .ITEM_BITS("Sp. Def Stage", 19, 0, 4)
     .ITEM_BITS("Speed Stage", 19, 4, 4)
     .ITEM_BITS("Accuracy Stage", 20, 0, 4)
     .ITEM_BITS("Critical Hit Stage", 20, 4, 2)

     .AddSection("Effort Values")
     .ITEM_BITS("Raises HP EV", 21, 3, 1)
     .Add("HP EV Change", data.hp_ev_value)
     .ITEM_BITS("Raises Attack EV", 21, 4, 1)
     .Add("Attack EV Change", data.attack_ev_value)
     .ITEM_BITS("Raises Defense EV", 21, 5, 1)
     .Add("Defense EV Change", data.defense_ev_value)
     .ITEM_BITS("Raises Sp. Atk EV", 21, 7, 1)
     .Add("Sp. Atk EV Change", data.sp_atk_ev_value)
     .ITEM_BITS("Raises Sp. Def EV", 22, 0, 1)
     .Add("Sp. Def EV Change", data.sp_def_ev_value)
     .ITEM_BITS("Raises Speed EV", 21, 6, 1)
     .Add("Speed EV Change", data.speed_ev_value)
     .ITEM_BITS("Follows The EV Limits", 22, 1, 1)

     .AddSection("Friendship")
     .ITEM_BITS("Changes Friendship (Low)", 22, 2, 1)
     .Add("Friendship Change (Low)", data.friendship_value_1)
     .ITEM_BITS("Changes Friendship (Medium)", 22, 3, 1)
     .Add("Friendship Change (Medium)", data.friendship_value_2)
     .ITEM_BITS("Changes Friendship (High)", 22, 4, 1)
     .Add("Friendship Change (High)", data.friendship_value_3);
}
#undef ITEM_BITS

void LoadGameDataPage(MainApplication& app, void* args) {
  app.AddSection("Pokemon")
     .Add("Species Data", LoadSpeciesDataPage)
     .WithDescription("Base stats, types, abilities and TMs of each species.")
     .Add("Move Data", LoadMoveDataPage)
     .WithDescription("Type, power, accuracy and effects of each move.")
     .Add("TMs and HMs", LoadTechnicalMachinePage)
     .WithDescription("The move of each TM and HM.")
     .AddSection("Items")
     .Add("Item Data", LoadItemDataPage)
     .WithDescription("Price, pocket, effects and boosts of each item.")
     .AddSection("Battle")
     .Add("Type Chart", LoadTypeChartPage)
     .WithDescription("Changes the effectiveness of one type against "
                      "another type.");
}
} // namespace ui
