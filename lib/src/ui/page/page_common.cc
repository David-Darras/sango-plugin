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
 * @file page_common.cc
 * @brief The functions and the texts that several menu pages share.
 */

#include "ui/page/page_common.h"

#include <cstdio>

#include "core/native/data_manager.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/species_data.h"
#include "pokemon/native/technical_machine_table.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "ui/main_application.h"

namespace ui {
const c8* kGenderNames[3] = {"Male", "Female", "Genderless"};

const c8* kNatureNames[25] = {
    "Hardy", "Lonely", "Brave",   "Adamant", "Naughty", "Bold",    "Docile",
    "Relaxed", "Impish", "Lax",   "Timid",   "Hasty",   "Serious", "Jolly",
    "Naive", "Modest", "Mild",    "Quiet",   "Bashful", "Rash",    "Calm",
    "Gentle", "Sassy", "Careful", "Quirky"};

const c8* kBallNames[26] = {
    "None",        "Master Ball", "Ultra Ball",  "Great Ball", "Poke Ball",
    "Safari Ball", "Net Ball",    "Dive Ball",   "Nest Ball",  "Repeat Ball",
    "Timer Ball",  "Luxury Ball", "Premier Ball", "Dusk Ball", "Heal Ball",
    "Quick Ball",  "Cherish Ball", "Fast Ball",  "Level Ball", "Lure Ball",
    "Heavy Ball",  "Love Ball",   "Friend Ball", "Moon Ball",  "Sport Ball",
    "Dream Ball"};

const c8* kLanguageNames[9] = {"None",   "Japanese", "English",
                               "French", "Italian",  "German",
                               "---",    "Spanish",  "Korean"};

const c8* kStatStageNames[13] = {"-6", "-5", "-4", "-3", "-2", "-1", "0",
                                 "+1", "+2", "+3", "+4", "+5", "+6"};

const c8* kGameVersionNames[28] = {
    "---",       "Sapphire", "Ruby",     "Emerald",   "FireRed",
    "LeafGreen", "---",      "HeartGold", "SoulSilver", "---",
    "Diamond",   "Pearl",    "Platinum", "---",       "---",
    "Colosseum / XD", "---", "---",      "---",       "---",
    "White",     "Black",    "White 2",  "Black 2",   "X",
    "Y",         "Alpha Sapphire", "Omega Ruby"};

const c8* kSideNames[4] = {"Up", "Down", "Left", "Right"};

namespace {
// The TMs of the table of the TMs: TM01 to TM100.
constexpr u32 kTechnicalMachineCount = 100;

// Adds a value to a list, without the same value two times.
void AddUnique(u16 value, u16* ids, u32 capacity, u32& count) {
  if (value == 0 || count >= capacity) return;
  for (u32 i = 0; i < count; i++) {
    if (ids[i] == value) return;
  }
  ids[count++] = value;
}
} // namespace

u32 GetSpeciesAbilities(SpeciesId species, FormId form, u16* ids,
                        u32 capacity) {
  if (species == SpeciesId::kNone) return 0;
  const pokemon::SpeciesData& data =
      pokemon::SpeciesData::GetInstance(species, form);
  u32 count = 0;
  for (u32 i = 0; i < 3; i++) {
    AddUnique(static_cast<u16>(data.ability[i]), ids, capacity, count);
  }
  return count;
}

u32 GetSpeciesMoves(SpeciesId species, FormId form, u16* ids, u32 capacity) {
  if (species == SpeciesId::kNone) return 0;
  u32 count = 0;
  pokemon::Movepool& movepool = pokemon::Movepool::GetInstance(species, form);
  for (u32 i = 0; i < movepool.count; i++) {
    AddUnique(static_cast<u16>(movepool.entry[i].move), ids, capacity, count);
  }
  const pokemon::SpeciesData& data =
      pokemon::SpeciesData::GetInstance(species, form);
  const MoveId* machines = pokemon::TechnicalMachineTable::GetTable();
  for (u32 i = 0; i < kTechnicalMachineCount; i++) {
    if ((data.technical_moves[i / 32] >> (i % 32)) & 1) {
      AddUnique(static_cast<u16>(machines[i]), ids, capacity, count);
    }
  }
  return count;
}

const c8** GetNumberedNames(const c8* format, u32 count) {
  struct List {
    const c8* format;
    u32 count;
    const c8** names;
  };
  // One list for each call site of the pages: less than 32.
  static List lists[32];
  static u32 list_count = 0;
  if (count > 100) count = 100;

  for (u32 i = 0; i < list_count; i++) {
    if (lists[i].format == format && lists[i].count == count) {
      return lists[i].names;
    }
  }

  // Make the texts one time. They stay in memory.
  constexpr u32 kLength = 16;
  c8* texts = new c8[count * kLength];
  const c8** names = new const c8*[count];
  for (u32 i = 0; i < count; i++) {
    snprintf(texts + i * kLength, kLength, format, i + 1);
    names[i] = texts + i * kLength;
  }
  if (list_count < SIZE(lists)) {
    lists[list_count++] = List{format, count, names};
  }
  return names;
}

void LoadColorPage(MainApplication& app, void* args) {
  constexpr f32 kFactor = 0.025f;
  Color& color = *(Color*)args;

  app.Add("Red", color.r).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Green", color.g).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Blue", color.b).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Alpha", color.a).WithFactor(kFactor).WithBounds(0, 1);
}

void LoadColor8Page(MainApplication& app, void* args) {
  Color8& color = *(Color8*)args;

  app.Add("Red", color.r)
      .Add("Green", color.g)
      .Add("Blue", color.b)
      .Add("Alpha", color.a);
}

void RefreshMap(void*) {
  auto& main_app = MainApplication::GetInstance();
  if (main_app.CheckProcess(overworld::address::kVtable)) return;

  const overworld::Position& pos =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  const bool same_background_music = true;
  const bool show_map_name = false;
  overworld::Facing facing = core::DataManager::GetInstance().GetPlayerDirection();
  if (facing >= overworld::Facing::kCount) facing = overworld::Facing::kUp;
  overworld::MapManager::ChangeMap(overworld::MapManager::GetInstance().GetMap(),
                                   pos, facing, same_background_music,
                                   show_map_name);

  main_app.ForceClose();
}
} // namespace ui
