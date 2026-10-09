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
 * @file page_save_data.cc
 * @brief The menu pages of the Save Data family.
 */

#include <cstring>

#include "overworld/native/berry_tree_location.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/data_accessor.h"
#include "pokemon/native/utils.h"
#include "savedata/native/bag_manager.h"
#include "savedata/native/battle_box.h"
#include "savedata/native/berry_tree_manager.h"
#include "savedata/native/box_manager.h"
#include "savedata/native/day_care.h"
#include "savedata/native/fusion.h"
#include "savedata/native/hall_of_fame.h"
#include "savedata/native/item_manager.h"
#include "savedata/native/minigame.h"
#include "savedata/native/misc.h"
#include "savedata/native/opower_manager.h"
#include "savedata/native/overworld_menu.h"
#include "savedata/native/play_time.h"
#include "savedata/native/pokedex.h"
#include "savedata/native/pokemon_amie.h"
#include "savedata/native/pokemon_box.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/pss.h"
#include "savedata/native/record_manager.h"
#include "savedata/native/repel.h"
#include "savedata/native/settings.h"
#include "savedata/native/trainer_status.h"
#include "ui/main_application.h"

#include "savedata/native/pss_group.h"
#include "system/native/core.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
// The Pokemon of the save data are encrypted. The page shows a decrypted
// copy; SavePokemon() encrypts the copy and writes it back after each change.
static struct {
  u8 level;
  bool is_shiny;
  pokemon::DataAccessor accessor;
  pokemon::CoreData backup_core_data;
  pokemon::CoreData* core_data;
} ctx;

static void SavePokemon(void*) {
  pokemon::CoreData* pkm = ctx.accessor.GetCoreData();

  pkm->experience =
      pokemon::Utils::GetExperienceFromLevel(pkm->species, pkm->form,
                                             ctx.level);
  if (ctx.is_shiny) {
    pokemon::Utils::ConvertToShiny(pkm->id, &pkm->shiny_id);
  } else {
    pokemon::Utils::ConvertToNormal(pkm->id, &pkm->shiny_id);
  }

  ctx.accessor.Encrypt();
  std::memcpy(ctx.core_data, &ctx.backup_core_data,
              sizeof(ctx.backup_core_data));
  auto& team = savedata::PokemonTeam::GetInstance();
  for (u32 i = 0; i < team.count; i++) {
    if (team.pokemons[i]->core == ctx.core_data) {
      team.pokemons[i]->UpdateRuntimeData();
    }
  }
  ctx.accessor.Decrypt();
}

void LoadSaveDataPokemonPage(MainApplication& app, void* args) {
  ctx.core_data = (pokemon::CoreData*)args;
  std::memcpy(&ctx.backup_core_data, ctx.core_data,
              sizeof(ctx.backup_core_data));

  ctx.accessor.Initialize(&ctx.backup_core_data, nullptr);
  ctx.accessor.Decrypt();

  pokemon::CoreData* pkm = ctx.accessor.GetCoreData();

  if (pkm->species == SpeciesId::kNone) {
    app.Add("There is no Pokemon in this slot.");
    return;
  }

  ctx.level = pokemon::Utils::GetLevelFromExperience(pkm->species, pkm->form,
    pkm->experience);
  ctx.is_shiny = pokemon::Utils::IsShiny(pkm->id, pkm->shiny_id);

  app.OnChange(SavePokemon)
     .AddSection("Pokemon")
     .AddSpecies("Species", pkm->species)
     .Add("Form", &pkm->event_gender_form_flags, 3, 5)
     .Add("Nickname", pkm->nickname, 13)
     .Add("Level", ctx.level)
     .WithBounds(1, 100)
     .Add("Shiny", ctx.is_shiny)
     .Add("Gender", &pkm->event_gender_form_flags, 1, 2)
     .WithArray(kGenderNames, SIZE(kGenderNames))
     .Add("Nature", pkm->nature)
     .WithArray(kNatureNames, SIZE(kNatureNames))
     .AddAbility("Ability", pkm->ability)
     .AddItem("Held Item", pkm->item)
     .Add("Is Egg", &pkm->iv_flags, 30, 1)
     .WithRefresh();

  if ((pkm->iv_flags >> 30) & 1) {
    app.Add("Egg Steps", pkm->remaining_steps_before_hatch)
       .WithDescription("The number of step cycles before the Egg hatches.");
  }

  app.AddSection("Moves")
     .AddMove("Move 1", pkm->moves[0])
     .Add("PP 1", pkm->pp[0])
     .AddMove("Move 2", pkm->moves[1])
     .Add("PP 2", pkm->pp[1])
     .AddMove("Move 3", pkm->moves[2])
     .Add("PP 3", pkm->pp[2])
     .AddMove("Move 4", pkm->moves[3])
     .Add("PP 4", pkm->pp[3]);

  app.AddSection("Effort Values")
     .Add("EV HP", pkm->ev_hp)
     .Add("EV Attack", pkm->ev_attack)
     .Add("EV Defense", pkm->ev_defense)
     .Add("EV Sp. Atk", pkm->ev_special_attack)
     .Add("EV Sp. Def", pkm->ev_special_defense)
     .Add("EV Speed", pkm->ev_speed);

  app.AddSection("Individual Values")
     .Add("IV HP", &pkm->iv_flags, 0, 5)
     .Add("IV Attack", &pkm->iv_flags, 5, 5)
     .Add("IV Defense", &pkm->iv_flags, 10, 5)
     .Add("IV Sp. Atk", &pkm->iv_flags, 20, 5)
     .Add("IV Sp. Def", &pkm->iv_flags, 25, 5)
     .Add("IV Speed", &pkm->iv_flags, 15, 5);

  app.AddSection("Origin")
     .Add("Poke Ball", pkm->ball)
     .WithArray(kBallNames, SIZE(kBallNames))
     .Add("Original Trainer", pkm->original_trainer_name, 13)
     .Add("Language", pkm->language)
     .WithArray(kLanguageNames, SIZE(kLanguageNames));

  app.AddSection("Contest")
     .Add("Cool", pkm->contest.cool)
     .Add("Beauty", pkm->contest.beautiful)
     .Add("Cute", pkm->contest.cute)
     .Add("Clever", pkm->contest.smart)
     .Add("Tough", pkm->contest.tough)
     .Add("Sheen", pkm->contest.sheen);
}

void LoadSaveDataTeamPage(MainApplication& app, void* args) {
  static u8 slot_idx = 0;
  auto& data = savedata::PokemonTeam::GetInstance();

  app.AddSection("Team")
     .Add("Slot", slot_idx)
     .WithArray(GetNumberedNames("Slot %u", 6), 6)
     .WithRefresh()
     .Add("Pokemon Count", data.count)
     .WithBounds(0, 6)
     .WithDescription("The number of Pokemon in the team. Change it with "
                      "care.");

  LoadSaveDataPokemonPage(app, data.pokemons[slot_idx]->core);
}

void LoadSaveDataBattleBoxPage(MainApplication& app, void* args) {
  static u8 slot_idx = 0;
  auto& data = savedata::BattleBox::GetInstance();

  app.AddSection("Battle Box")
     .Add("Slot", slot_idx)
     .WithArray(GetNumberedNames("Slot %u", savedata::BattleBox::kMaxSlots),
                savedata::BattleBox::kMaxSlots)
     .WithRefresh();

  LoadSaveDataPokemonPage(app, &data.pokemons[slot_idx]);
}

void LoadSaveDataPokemonBoxPage(MainApplication& app, void* args) {
  static u8 box_idx = 0;
  static u8 slot_idx = 0;
  constexpr u32 kBoxes = savedata::PokemonBox::kMaxBoxes;
  constexpr u32 kSlots = savedata::PokemonBox::kMaxSlotsPerBox;

  auto& data = savedata::PokemonBox::GetInstance();
  app.AddSection("PC Box")
     .Add("Box", box_idx)
     .WithArray(GetNumberedNames("Box %u", kBoxes), kBoxes)
     .WithRefresh()
     .Add("Slot", slot_idx)
     .WithArray(GetNumberedNames("Slot %u", kSlots), kSlots)
     .WithRefresh();

  LoadSaveDataPokemonPage(app, &data.boxes[box_idx].pokemons[slot_idx]);
}

void LoadSaveDataDayCarePage(MainApplication& app, void* args) {
#ifdef GAME_ORAS
  static const c8* LOCATIONS[] = {"Route 117", "Battle Resort"};
#else
  static const c8* LOCATIONS[] = {"Route 7"};
#endif
  static_assert(SIZE(LOCATIONS) == savedata::DayCare::kLocationCount,
                "one name for each Day Care");
  static u32 loc = 0;
  static u32 idx = 0;

  auto& day_care = savedata::DayCare::GetInstance();
  pokemon::CoreData* data = &day_care.location[loc].pokemon[idx].data;

  app.AddSection("Day Care")
     .Add("Day Care", loc)
     .WithArray(LOCATIONS, SIZE(LOCATIONS))
     .WithRefresh()
     .Add("Pokemon", idx)
     .WithArray(GetNumberedNames("Pokemon %u", 2), 2)
     .WithRefresh()
     .Add("Egg Is Ready", day_care.location[loc].is_egg_available);

  LoadSaveDataPokemonPage(app, data);
}

void LoadSaveDataBoxesMetadataPage(MainApplication& app, void* args) {
  static u8 index = 0;
  constexpr u32 kBoxes = savedata::BoxManager::kMaxBoxes;
  auto& data = savedata::BoxManager::GetInstance();

  app.AddSection("Box")
     .Add("Box", index)
     .WithArray(GetNumberedNames("Box %u", kBoxes), kBoxes)
     .WithRefresh()
     .Add("Name", data.titles[index], savedata::BoxManager::kMaxTitleLength)
     .Add("Wallpaper", data.wallpapers[index])
     .WithBounds(0, 23)
     .AddSection("All The Boxes")
     .Add("Unlocked Boxes", data.unlocked_count)
     .Add("Active Box", data.active_box_index)
     .WithArray(GetNumberedNames("Box %u", kBoxes), kBoxes)
     .Add("Special Wallpapers Unlocked", &data.flags, 0, 7)
     .Add("Event Box Open", &data.flags, 7, 1);
}

void LoadSaveDataBagItemsPage(MainApplication& app, void* args) {
  static u32 pocket_id = 0;
  static u32 slot_idx = 0;

  static const c8* pocket_names[] = {
      "Items", "Key Items", "TMs & HMs",
      "Medicine", "Berries"
  };

  auto& data = savedata::ItemManager::GetInstance();
  savedata::ItemManager::ItemSlot* target_pocket = nullptr;
  u32 max_slots = 0;

  switch (pocket_id) {
    case 1:
      target_pocket = data.GetKeyItems();
      max_slots = savedata::ItemManager::kMaxKeyItems;
      break;
    case 2:
      target_pocket = data.GetTMsHMs();
      max_slots = savedata::ItemManager::kMaxTMsHMs;
      break;
    case 3:
      target_pocket = data.GetMedicine();
      max_slots = savedata::ItemManager::kMaxMedicine;
      break;
    case 4:
      target_pocket = data.GetBerries();
      max_slots = savedata::ItemManager::kMaxBerries;
      break;
    case 0:
    default:
      target_pocket = data.GetNormalItems();
      max_slots = savedata::ItemManager::kMaxNormalItems;
      break;
  }
  if (slot_idx >= max_slots) slot_idx = 0;

  app.AddSection("Bag")
     .Add("Pocket", pocket_id)
     .WithArray(pocket_names, SIZE(pocket_names))
     .WithRefresh()
     .Add("Slot", slot_idx)
     .WithBounds(0, max_slots - 1)
     .WithRefresh()
     .WithDescription("The place of the item in the pocket. 0 is the first "
                      "item.")
     .AddSection("Item")
     .AddItem("Item", target_pocket[slot_idx].id)
     .Add("Quantity", target_pocket[slot_idx].count)
     .WithBounds(0, savedata::ItemManager::kMaxItemCount);
}

void LoadSaveDataBagMetadataPage(MainApplication& app, void* args) {
  static u8 pocket_idx = 0;
  static u8 register_idx = 0;
  static u8 history_idx = 0;
  static const c8* pocket_type[savedata::BagManager::kMaxPockets] = {
      "Items", "Medicine", "TMs & HMs",
      "Berries", "Key Items"
  };

  auto& data = savedata::BagManager::GetInstance();

  app.AddSection("Pocket Order")
     .Add("Position", pocket_idx)
     .WithArray(GetNumberedNames("Position %u",
                                 savedata::BagManager::kMaxPockets),
                savedata::BagManager::kMaxPockets)
     .WithRefresh()
     .Add("Pocket", data.pocket_order[pocket_idx])
     .WithArray(pocket_type, savedata::BagManager::kMaxPockets)
     .AddSection("Registered Items")
     .Add("Shortcut", register_idx)
     .WithArray(GetNumberedNames("Shortcut %u",
                                 savedata::BagManager::kMaxRegisteredItems),
                savedata::BagManager::kMaxRegisteredItems)
     .WithRefresh()
     .AddItem("Registered Item", data.registered_items[register_idx])
     .AddSection("Last Used Items")
     .Add("History", history_idx)
     .WithArray(GetNumberedNames("Item %u",
                                 savedata::BagManager::kMaxUsageHistory),
                savedata::BagManager::kMaxUsageHistory)
     .WithRefresh()
     .AddItem("Last Item Used", data.last_items_used[history_idx]);
}

#include "savedata/data/records.inc"

void LoadSaveDataRecordsPage(MainApplication& app, void* args) {
  static u32 record_0_idx = 0;
  static u32 record_1_idx = 0;

  auto& data = savedata::RecordManager::GetInstance();

  app.Add("Records Are Disabled", data.is_disabled)
     .AddSection("Large Records")
     .Add("Record", record_0_idx)
     .WithArray(RECORDS_0, SIZE(RECORDS_0))
     .WithRefresh()
     .Add("Value", data.records_0[record_0_idx])
     .AddSection("Small Records")
     .Add("Record", record_1_idx)
     .WithArray(RECORDS_1, SIZE(RECORDS_1))
     .WithRefresh()
     .Add("Value", data.records_1[record_1_idx]);
}

void LoadSaveDataMiscellaneousPage(MainApplication& app, void* args) {
  auto& data = savedata::Misc::GetInstance();

  app.AddSection("Money")
     .Add("Money", data.money)
     .Add("Battle Points", data.battle_points)
     .AddSection("Badges");

  for (u32 i = 0; i < 8; i++) {
    app.Add(GetNumberedNames("Badge %u", 8)[i], &data.badges, i, 1);
  }

  app.AddSection("Other")
     .Add("Rival Nickname", data.rival_nickname,
          savedata::Misc::kNicknameLength)
     .Add("Unlock Pokémon League Wallpapers", &data.flags, 0, 1)
     .Add("Keyboard Layout", &data.flags, 2, 1)
     .Add("Exp. Share Enabled", &data.flags, 3, 1)
#ifdef GAME_XY
     .Add("Pokemon-Amie Tutorial Seen", &data.tutorial_pokemon_amie, 0, 1)
      .Add("Super Training Tutorial Seen", &data.tutorial_super_training, 0, 1)
      .Add("Vs. Recorder Tutorial Seen", &data.flags2, 1, 1);
#else
      .Add("PSS Tutorial Seen", &data.flags, 5, 1)
      .Add("Pokemon-Amie Tutorial Seen", &data.flags, 6, 1)
      .Add("Super Training Tutorial Seen", &data.flags, 7, 1)
      .Add("Vs. Recorder Tutorial Seen", &data.flags, 9, 1)
      .Add("Skip Long Sky Trip Animation", &data.flags, 11, 1)
      .Add("TV Navi Tutorial Seen", &data.flags, 14, 1);
#endif
}

void LoadSaveDataTrainerStatusPage(MainApplication& app, void* args) {
  static const c8* GENDERS[] = {"Male", "Female"};
  auto& data = savedata::TrainerStatus::GetInstance();

  app.AddSection("Trainer")
     .Add("Player Name", data.name, savedata::TrainerStatus::kPlayerNameLen)
     .Add("Nickname", data.nickname, savedata::TrainerStatus::kPlayerNameLen)
     .Add("Trainer ID (TID)", data.trainer_id)
     .WithDescription("The Trainer ID that the Trainer Card shows.")
     .Add("Secret ID (SID)", data.secret_id)
     .WithDescription("The hidden part of the Trainer ID. It changes the "
                      "shiny Pokemon.")
     .Add("Gender", data.gender)
     .WithArray(GENDERS, SIZE(GENDERS))
     .Add("Game Version", data.game_version)
     .WithArray(kGameVersionNames, SIZE(kGameVersionNames))

     .AddSection("Mega Evolution")
     .Add("Mega Ring Obtained", &data.mega_flags, 0, 1)
     .Add("Mega Rayquaza Unlocked", &data.mega_flags, 1, 1)

     .AddSection("PSS")
     .Add("PSS Icon", data.pss_icon)
     .Add("PSS Message 1", data.pss_messages[0],
          savedata::TrainerStatus::kPssMessageLen)
     .Add("PSS Message 2", data.pss_messages[1],
          savedata::TrainerStatus::kPssMessageLen)
     .Add("PSS Message 3", data.pss_messages[2],
          savedata::TrainerStatus::kPssMessageLen)
     .Add("PSS Message 4", data.pss_messages[3],
          savedata::TrainerStatus::kPssMessageLen)
     .Add("PSS Message 5", data.pss_messages[4],
          savedata::TrainerStatus::kPssMessageLen)
     .Add("PSS Message 6", data.pss_messages[5],
          savedata::TrainerStatus::kPssMessageLen)
     .Add("Reject Friend Requests", &data.pss_flags, 0, 1)
     .Add("Reject Acquaintance Requests", &data.pss_flags, 1, 1)
     .Add("Reject Passersby Requests", &data.pss_flags, 2, 1)
     .Add("Reject Voice Chat", &data.pss_flags, 3, 1)
     .Add("Reject PR Video Exchange", &data.pss_flags, 4, 1)

     .AddSection("Console")
     .Add("Region", data.region)
     .Add("Latitude", data.latitude)
     .Add("Longitude", data.longitude)
     .Add("NEX ID", data.nex_id)
     .Add("Principal ID", data.principal_id)
     .Add("Current Console ID", data.current_console_id)
     .Add("Original Console ID", data.original_console_id)
     .Add("PSS ID", data.pss_id)
     .Add("COPPA Restriction", data.coppa_restriction);
}

void LoadSaveDataOverworldMenuPage(MainApplication& app, void* args) {
  auto& data = savedata::OverworldMenu::GetInstance();

  app.AddSection("Pokemon")
     .Add("Visible", &data.flags, 0, 1)
     .Add("Position", &data.flags, 6, 3)
     .AddSection("Pokedex")
     .Add("Visible", &data.flags, 1, 1)
     .Add("Position", &data.flags, 9, 3)
     .AddSection("Bag")
     .Add("Visible", &data.flags, 2, 1)
     .Add("Position", &data.flags, 12, 3)
     .AddSection("Trainer Card")
     .Add("Visible", &data.flags, 3, 1)
     .Add("Position", &data.flags, 15, 3)
     .AddSection("Save")
     .Add("Visible", &data.flags, 4, 1)
     .Add("Position", &data.flags, 18, 3)
     .AddSection("Options")
     .Add("Visible", &data.flags, 5, 1)
     .Add("Position", &data.flags, 21, 3);
}

void LoadSaveDataMinigamePage(MainApplication& app, void* args) {
  static u32 choice = 0;
  static u32 puzzle_idx = 0;

  static const char* DIFFICULTIES[] = {
      "Easy", "Normal", "Hard", "Unlimited"
  };

  static const char* RATINGS[] = {
      "None", "1 Star", "2 Stars", "3 Stars", "4 Stars", "4.5 Stars", "5 Stars"
  };

  auto& data = savedata::Minigame::GetInstance();
  auto& puzzle = data.tile_puzzle_scores[choice];

  app.Add("Difficulty", choice)
     .WithArray(DIFFICULTIES, SIZE(DIFFICULTIES))
     .WithRefresh()
     .AddSection("Berry Picker")
     .Add("Score", data.berry_picker_high_scores[choice])
     .WithBounds(0, 999)
     .Add("Rating", data.berry_picker_best_ratings[choice])
     .WithArray(RATINGS, SIZE(RATINGS))
     .AddSection("Head It")
     .Add("Score", data.head_it_high_scores[choice])
     .WithBounds(0, 9999)
     .Add("Rating", data.head_it_best_ratings[choice])
     .WithArray(RATINGS, SIZE(RATINGS))
     .AddSection("Tile Puzzle")
     .Add("Puzzle", puzzle_idx)
     .WithArray(GetNumberedNames("Puzzle %u", 5), 5)
     .WithRefresh()
     .Add("Score", puzzle.total_score[puzzle_idx])
     .Add("Time (s)", puzzle.time[puzzle_idx])
     .Add("Moves", puzzle.moves[puzzle_idx])
     .Add("Swaps", puzzle.swaps[puzzle_idx])
     .Add("Rating", data.tile_puzzle_best_ratings[choice])
     .WithArray(RATINGS, SIZE(RATINGS));
}

void LoadSaveDataPokemonAmiePage(MainApplication& app, void* args) {
  static u8 puff_idx = 0;
  auto& data = savedata::PokemonAmie::GetInstance();

  app.AddSection("Poke Puffs")
     .Add("Slot", puff_idx)
     .WithBounds(0, savedata::PokemonAmie::kMaxPokePuffs - 1)
     .WithRefresh()
     .Add("Poke Puff", data.poke_puffs[puff_idx])
     .WithBounds(0, savedata::PokemonAmie::kMaxPokePuffId)
     .WithDescription("The kind of the Poke Puff. 0: no Poke Puff.")
     .AddSection("Other")
     .Add("Last Opened (Days)", data.last_opened_timestamp);
}

#include "savedata/data/pokedex_form.inc"

void LoadSaveDataPokedexPage(MainApplication& app, void* args) {
  static u16 species = 1;
  static u8 form = 0;
  static u16 prev_species = species;
  if (species != prev_species) {
    form = 0;
    prev_species = species;
  }

  u32 idx, bit_pos, array_idx;
  auto& data = savedata::Pokedex::GetInstance();

  app.AddSpecies("Species", species)
     .WithRefresh();

  idx = species - 1;
  bit_pos = idx & 31;
  array_idx = idx >> 5;

  app.Add("Caught", &data.captured_flags[array_idx], bit_pos, 1)
#ifndef GAME_XY
      .Add("Times Encountered", data.seen_count[species])
      .WithBounds(0, 999)
#endif
      .AddSection("Seen")
      .Add("Male", &data.gender_seen_flags[0][array_idx], bit_pos, 1)
      .Add("Female", &data.gender_seen_flags[1][array_idx], bit_pos, 1)
      .Add("Shiny Male", &data.gender_seen_flags[2][array_idx], bit_pos, 1)
      .Add("Shiny Female", &data.gender_seen_flags[3][array_idx], bit_pos, 1)
      .AddSection("Shown In The Pokedex")
      .Add("Male", &data.displayed_gender_flags[0][array_idx], bit_pos, 1)
      .Add("Female", &data.displayed_gender_flags[1][array_idx], bit_pos, 1)
      .Add("Shiny Male", &data.displayed_gender_flags[2][array_idx], bit_pos,
           1)
      .Add("Shiny Female", &data.displayed_gender_flags[3][array_idx],
           bit_pos, 1);

  s32 form_index = data.GetFormIndex(species);
  if (form_index >= 0) {
    idx = (u32)form_index;
    s32 table_index, form_max;
    savedata::Pokedex::GetTableIndexAndFormMax(table_index, form_max, species);
    if (form >= form_max) form = 0;

    idx += form;
    bit_pos = idx & 31;
    array_idx = idx >> 5;

    u32* normal_form_seen_flags = (u32*)data.form_seen_flags[0];
    u32* shiny_form_seen_flags = (u32*)data.form_seen_flags[1];
    u32* normal_displayed_form_flags = (u32*)data.displayed_form_flags[0];
    u32* shiny_displayed_form_flags = (u32*)data.displayed_form_flags[1];

    app.AddSection("Forms")
       .Add("Form", form)
       .WithArray(FORMS[table_index], form_max)
       .WithRefresh()
       .Add("Seen", &normal_form_seen_flags[array_idx], bit_pos, 1)
       .Add("Seen Shiny", &shiny_form_seen_flags[array_idx], bit_pos, 1)
       .Add("Shown", &normal_displayed_form_flags[array_idx], bit_pos, 1)
       .Add("Shown Shiny", &shiny_displayed_form_flags[array_idx], bit_pos,
            1);
  }

  app.AddSection("All The Pokedex")
     .Add("Spinda Pattern", data.spinda_pattern)
     .WithDescription("The spots of the Spinda that the Pokedex shows.");
}

#include "savedata/data/opower.inc"

void LoadSaveDataOPowerPage(MainApplication& app, void* args) {
  static u32 learned_opower_idx = 0;
  static u32 overworld_opower_idx = 0;
  static u32 battle_opower_idx = 0;

  auto& man = savedata::OPowerManager::GetInstance();

  app.Add("O-Power Points", man.power_points)
     .AddSection("Learned O-Powers")
     .Add("O-Power", learned_opower_idx)
     .WithArray(OPOWERS, SIZE(OPOWERS))
     .WithRefresh()
     .Add("Value", man.learned_powers[learned_opower_idx])
     .AddSection("Overworld O-Powers")
     .Add("O-Power", overworld_opower_idx)
     .WithArray(FIELD_OPOWERS, SIZE(FIELD_OPOWERS))
     .WithRefresh()
     .Add("Lv. 1 Uses",
          man.overworld_power_level_1_uses[overworld_opower_idx])
     .Add("Lv. 2 Uses",
          man.overworld_power_level_2_uses[overworld_opower_idx])
     .AddSection("Battle O-Powers")
     .Add("O-Power", battle_opower_idx)
     .WithArray(BATTLE_OPOWERS, SIZE(BATTLE_OPOWERS))
     .WithRefresh()
     .Add("Lv. 1 Uses", man.battle_power_level_1_uses[battle_opower_idx])
     .Add("Lv. 2 Uses", man.battle_power_level_2_uses[battle_opower_idx]);
}

void LoadSaveDataPlayTimePage(MainApplication& app, void* args) {
  auto& data = savedata::PlayTime::GetInstance();

  app.Add("Hours", data.hour)
     .Add("Minutes", data.minute)
     .WithBounds(0, 59)
     .Add("Seconds", data.second)
     .WithBounds(0, 59);
}

void OnUpdateLanguage(void*) {
  auto& settings = savedata::Settings::GetInstance();
  auto language = static_cast<Language>(settings.language);
  sys::Core::GetInstance().GetLanguage() = language;
  *(Language*)(ui::address::kLanguageId) = language;
  savedata::TrainerStatus::GetInstance().language = language;
}

void LoadSaveDataSettingsPage(MainApplication& app, void* args) {
  static const c8* TEXT_SPEED[] = {"Slow", "Normal", "Fast", "Instant"};
  static const c8* TOGGLE_OFF_ON[] = {"Off", "On"};
  static const c8* BATTLE_STYLE[] = {"Shift", "Set"};
  static const c8* BUTTON_MODE[] = {"Normal", "L=A", "LR Disabled"};
  static const c8* BATTLE_BACKGROUNDS[] = {
      "Default", "Red", "Blue", "Pikachu", "Starters",
      "Eevee", "Monochrome", "Stickers", "Tatami", "Floral Pattern",
      "Elegant", "Tall Grass", "Poke Ball", "Cockpit", "Carbon"
  };

  auto& settings = savedata::Settings::GetInstance();

  app.AddSection("Game")
     .Add("Text Speed", &settings.core, 0, 2)
     .WithArray(TEXT_SPEED, SIZE(TEXT_SPEED))
     .Add("Language", &settings.core, 4, 4)
     .WithArray(kLanguageNames, SIZE(kLanguageNames))
     .WithCallback(OnUpdateLanguage)
     .WithDescription("Select the language, then press A to use it. The "
                      "next texts use the new language.")
     .Add("Button Mode", &settings.core, 13, 2)
     .WithArray(BUTTON_MODE, SIZE(BUTTON_MODE))
     .AddSection("Battle")
     .Add("Battle Animations", &settings.core, 2, 1)
     .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))
     .Add("Battle Style", &settings.core, 3, 1)
     .WithArray(BATTLE_STYLE, SIZE(BATTLE_STYLE))
     .WithDescription("Shift: when a Pokemon of the opponent faints, the "
                      "game asks to switch Pokemon. Set: it does not ask.")
     .Add("Battle Background", &settings.core, 8, 5)
     .WithArray(BATTLE_BACKGROUNDS, SIZE(BATTLE_BACKGROUNDS))
     .AddSection("Communication")
     .Add("Save Before Communication", &settings.core, 15, 1)
     .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))
     .Add("SpotPass", &settings.core, 16, 1)
     .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))
     .Add("PSS", &settings.core, 17, 1)
     .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON));
}

void LoadSaveDataRepelPage(MainApplication& app, void* args) {
  auto& data = savedata::Repel::GetInstance();

  app.AddItem("Repel", data.spray_id)
     .Add("Steps Left", data.spray_count);
}

#include "savedata/data/pss.inc"

void LoadSaveDataPssProfilePage(MainApplication& app, void* args) {
  auto& profile = *(savedata::PssProfilePayload*)args;

  app.AddSection("Profile")
     .Add("Name", profile.name, 13)
     .Add("Shout-out Message", profile.shoutout_message, 17)
     .Add("Principal ID", profile.principal_id)
     .Add("Local Friend Code", profile.local_friend_code)
     .Add("Icon (X / Y)", &profile.flags, 8, 8)
     .WithArray(ICONS, SIZE(ICONS))
     .Add("Icon (OR / AS)", &profile.flags2, 13, 8)
     .WithArray(ICONS, SIZE(ICONS))
     .Add("Gender", &profile.flags, 16, 4)
     .Add("Region", profile.geographic_region_id)
     .Add("Birth Month", profile.birth_month)
     .WithBounds(1, 12)
     .Add("Birth Day", profile.birth_day)
     .WithBounds(1, 31)
     .Add("Game Version", profile.game_version)
     .WithArray(kGameVersionNames, SIZE(kGameVersionNames))
     .Add("Console Region", profile.console_region)

     .AddSection("State")
     .Add("Has Accepted EULA", &profile.flags, 20, 1)
     .Add("Has Promotion Video", &profile.flags, 21, 1)
     .Add("Has Pokemon In Party", &profile.flags, 22, 1)
     .Add("Has Hall Of Fame Completed", &profile.flags, 23, 1)
     .Add("Has Shout-Out Message", &profile.flags2, 1, 1)
     .Add("Meets Trade Conditions", &profile.flags2, 2, 1)

     .AddSection("Requests")
     .Add("Rejects Friend Requests", &profile.flags, 24, 1)
     .Add("Rejects Acquaintance Requests", &profile.flags, 25, 1)
     .Add("Rejects Passerby Requests", &profile.flags, 26, 1)
     .Add("Rejects Voice Chat", &profile.flags, 27, 1)
     .Add("Rejects Promo Video Requests", &profile.flags2, 0, 1)
     .Add("Rejects Lower Version Battles", &profile.flags2, 3, 1)
     .Add("Rejects Lower Version PR Videos", &profile.flags2, 4, 1)

     .AddSection("Parental Controls")
     .Add("Photo Sharing Disabled", &profile.flags, 28, 1)
     .Add("Internet Disabled", &profile.flags, 29, 1)
     .Add("Friend Registration Closed", &profile.flags, 30, 1)
     .Add("StreetPass Disabled", &profile.flags, 31, 1);
}

void LoadSaveDataPssGroupPage(MainApplication& app, void* args) {
  static u32 choice = 0;
  auto& grp = *(savedata::PssGroup*)args;
  app.Add("Profile", choice)
     .WithBounds(0, SIZE(grp.user_data) - 1)
     .WithRefresh();
  LoadSaveDataPssProfilePage(app, &grp.user_data[choice].datagram.profile);
}

void LoadSaveDataHallOfFamePage(MainApplication& app, void* args) {
  static const c8* OT_GENDERS[] = {"Male", "Female"};
  static u8 entry_idx = 0;
  static u8 slot_idx = 0;

  auto& data = savedata::HallOfFame::GetInstance();
  auto& entry = data.entries[entry_idx];
  auto* pkm = &entry.pokemon[slot_idx];

  app.AddSection("Hall of Fame")
     .Add("Entry", entry_idx)
     .WithArray(GetNumberedNames("Entry %u", 16), 16)
     .WithRefresh()
     .Add("Is Used", &entry.flags, 31, 1)
     .Add("Times Entered", &entry.flags, 0, 14)
     .Add("Year", &entry.flags, 14, 8)
     .Add("Month", &entry.flags, 22, 4)
     .WithBounds(1, 12)
     .Add("Day", &entry.flags, 26, 5)
     .WithBounds(1, 31)
     .Add("Slot", slot_idx)
     .WithArray(GetNumberedNames("Slot %u", 6), 6)
     .WithRefresh();

  if (pkm->species == SpeciesId::kNone) {
    app.Add("There is no Pokemon in this slot.");
    return;
  }

  app.AddSection("Pokemon")
     .AddSpecies("Species", pkm->species)
     .Add("Form", &pkm->flags, 0, 5)
     .Add("Gender", &pkm->flags, 5, 2)
     .WithArray(kGenderNames, SIZE(kGenderNames))
     .Add("Level", &pkm->flags, 7, 7)
     .WithBounds(1, 100)
     .Add("Shiny", &pkm->flags, 14, 1)
     .Add("Has Nickname", &pkm->flags, 15, 1)
     .Add("Nickname", pkm->nickname, 12)
     .AddItem("Held Item", pkm->item)
     .AddSection("Original Trainer")
     .Add("Name", pkm->trainer_name, 12)
     .Add("Gender", &pkm->flags, 16, 1)
     .WithArray(OT_GENDERS, SIZE(OT_GENDERS))
     .Add("ID 0", pkm->id0)
     .Add("ID 1", pkm->id1)
     .AddSection("Moves")
     .AddMove("Move 1", pkm->moves[0])
     .AddMove("Move 2", pkm->moves[1])
     .AddMove("Move 3", pkm->moves[2])
     .AddMove("Move 4", pkm->moves[3]);
}

static u32 tree_idx = 0;
static ItemId berry_item_id = ItemId::kNone;

static void UpdateBerryId(void*) {
  auto& data = savedata::BerryTreeManager::GetInstance();
  auto& tree = data.berry_trees[tree_idx];
  berry_item_id = savedata::BerryIdToItemId(tree.berry_id);
}

void LoadSaveDataBerryTreePage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  static const c8* BERRY_TREE_STATES[] = {
      "None", "Seeded", "Sprout", "Tall",
      "Flowering", "Berries", "Withered"
  };

  auto& data = savedata::BerryTreeManager::GetInstance();
  auto& tree = data.berry_trees[tree_idx];
  auto& loc = overworld::BerryTreeLocation::GetInstance(tree_idx);
  UpdateBerryId(nullptr);

  app.OnChange(UpdateBerryId)
     .Add("Tree", tree_idx)
     .WithBounds(0, savedata::BerryTreeManager::kMaxBerryTrees - 1)
     .WithRefresh()
     .AddSection("Berry")
     .Add("State", reinterpret_cast<u32&>(tree.state))
     .WithArray(BERRY_TREE_STATES, SIZE(BERRY_TREE_STATES))
     .Add("Berry ID", tree.berry_id)
     .AddItem("Berry", berry_item_id)
     .WithReadOnly()
     .Add("Count", tree.count)
     .Add("Elapsed Minutes", tree.elapsed_minutes)
     .Add("Moisture Minutes", tree.moisture_minutes)
     .Add("Use Default Berry", tree.use_default_berry)
     .AddSection("Place")
     .Add("Map ID", loc.map_id)
     .Add("Tile X", loc.tile_x)
     .Add("Tile Z", loc.tile_z)
     .Add("Height", loc.height);
}

// The top page of the save data: all the parts, in four sections.
void LoadSaveDataPage(MainApplication& app, void* args) {
  auto& sv = savedata::SaveData::GetInstance();
  auto& fusion = savedata::Fusion::GetInstance();

  app.AddSection("Pokemon")
     .Add("Team", LoadSaveDataTeamPage)
     .WithDescription("The Pokemon of the team. The changes apply at once.")
     .Add("PC Boxes", LoadSaveDataPokemonBoxPage)
     .WithDescription("The Pokemon of the PC. The changes apply at once.")
     .Add("Battle Box", LoadSaveDataBattleBoxPage)
     .Add("Day Care", LoadSaveDataDayCarePage)
     .Add("Fusion", LoadSaveDataPokemonPage, &fusion.pokemon)
     .WithDescription("The Pokemon inside a fused Kyurem (Reshiram or "
                      "Zekrom).")
     .Add("Pokedex", LoadSaveDataPokedexPage)
     .Add("Hall of Fame", LoadSaveDataHallOfFamePage)

     .AddSection("Trainer")
     .Add("Trainer Status", LoadSaveDataTrainerStatusPage)
     .WithDescription("The name, the IDs and the PSS profile of the player.")
     .Add("Money, Badges, Flags", LoadSaveDataMiscellaneousPage)
     .Add("Play Time", LoadSaveDataPlayTimePage)
     .Add("Records", LoadSaveDataRecordsPage)
     .Add("Settings", LoadSaveDataSettingsPage)
     .WithDescription("The options of the game: text speed, language, "
                      "battle style...")
     .Add("Overworld Menu", LoadSaveDataOverworldMenuPage)
     .WithDescription("The icons of the X menu: visible or not, and their "
                      "positions.")

     .AddSection("Items")
     .Add("Bag Items", LoadSaveDataBagItemsPage)
     .Add("Bag Order And Shortcuts", LoadSaveDataBagMetadataPage)
     .Add("Box Names And Wallpapers", LoadSaveDataBoxesMetadataPage)
     .Add("Berry Trees", LoadSaveDataBerryTreePage)
     .Add("Repel", LoadSaveDataRepelPage)

     .AddSection("Activities")
     .Add("O-Power", LoadSaveDataOPowerPage)
     .Add("Pokemon-Amie", LoadSaveDataPokemonAmiePage)
     .Add("Minigames", LoadSaveDataMinigamePage)
     .Add("PSS Favorites", LoadSaveDataPssGroupPage,
          &sv.GetPssFavouriteGroup())
     .Add("PSS Friends", LoadSaveDataPssGroupPage, &sv.GetPssFriendGroup())
     .Add("PSS Acquaintances", LoadSaveDataPssGroupPage,
          &sv.GetPssAcquaintanceGroup());
}
} // namespace ui
