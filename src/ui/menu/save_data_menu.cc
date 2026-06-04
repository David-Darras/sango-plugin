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

#include <cstring>

#include "ui/plugin_menu.h"
#include "game/savedata/bag_manager.h"
#include "game/savedata/battle_box.h"
#include "game/savedata/box_manager.h"
#include "game/savedata/item_manager.h"
#include "game/savedata/minigame.h"
#include "game/savedata/misc.h"
#include "game/savedata/opower_manager.h"
#include "game/savedata/overworld_menu.h"
#include "game/savedata/play_time.h"
#include "game/savedata/pokedex.h"
#include "game/savedata/pokemon_amie.h"
#include "game/savedata/pokemon_box.h"
#include "game/savedata/pokemon_core_data.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/pokemon_utils.h"
#include "game/savedata/pss.h"
#include "game/savedata/record_manager.h"
#include "game/savedata/savedata_encounter.h"
#include "game/savedata/settings.h"
#include "game/savedata/trainer_status.h"

namespace ui {
static struct {
  u8 level;
  bool is_shiny;
  PokemonDataAccessor accessor;
  PokemonCoreData backup_core_data;
  PokemonCoreData* core_data;
} ctx;

static void SavePokemon(void*) {
  PokemonCoreData* pkm = ctx.accessor.GetCoreData();

  pkm->experience =
      PokemonUtils::GetExperienceFromLevel(pkm->species, pkm->form, ctx.level);
  if (ctx.is_shiny) {
    PokemonUtils::ConvertToShiny(pkm->id, &pkm->shiny_id);
  } else {
    PokemonUtils::ConvertToNormal(pkm->id, &pkm->shiny_id);
  }

  ctx.accessor.Encrypt();
  std::memcpy(ctx.core_data, &ctx.backup_core_data,
              sizeof(ctx.backup_core_data));
  ctx.accessor.Decrypt();
}

void LoadSaveDataPokemonMenu(PluginMenu& menu, void* args) {
  ctx.core_data = (PokemonCoreData*)args;
  std::memcpy(&ctx.backup_core_data, ctx.core_data,
              sizeof(ctx.backup_core_data));

  ctx.accessor.Initialize(&ctx.backup_core_data, nullptr);
  ctx.accessor.Decrypt();

  PokemonCoreData* pkm = ctx.accessor.GetCoreData();

  ctx.level = PokemonUtils::GetLevelFromExperience(pkm->species, pkm->form,
                                                   pkm->experience);
  ctx.is_shiny = PokemonUtils::IsShiny(pkm->id, pkm->shiny_id);

  menu.Add("Save", SavePokemon)
      .AddSeparator()
      .AddSpecies("Species", pkm->species)
      .Add("Is Shiny", ctx.is_shiny)
      .Add("Nickname", pkm->nickname, 13)
      .Add("Gender", &pkm->event_gender_form_flags, 1, 2)
      .Add("Form", &pkm->event_gender_form_flags, 3, 5)
      .Add("Level", ctx.level)
      .Add("Nature", pkm->nature)
      .AddAbility("Ability", pkm->ability)
      .AddItem("Item", pkm->item)
      .Add("Ball", pkm->ball)
      .Add("O.T", pkm->original_trainer_name, 13)
      .Add("Language", pkm->language)
      .Add("In egg", &pkm->iv_flags, 30, 1);

  if ((pkm->iv_flags >> 30) & 1) {
    menu.Add("Remaining Steps Before Hatch", pkm->remaining_steps_before_hatch);
  }

  menu.AddSeparator();

  menu.AddMove("Move 1", pkm->moves[0])
      .AddMove("Move 2", pkm->moves[1])
      .AddMove("Move 3", pkm->moves[2])
      .AddMove("Move 4", pkm->moves[3])
      .AddSeparator();

  menu.Add("PP 1", pkm->pp[0])
      .Add("PP 2", pkm->pp[1])
      .Add("PP 3", pkm->pp[2])
      .Add("PP 4", pkm->pp[3])
      .AddSeparator();

  menu.Add("EV - HP", pkm->ev.hp)
      .Add("EV - Attack", pkm->ev.attack)
      .Add("EV - Defense", pkm->ev.defense)
      .Add("EV - Speed", pkm->ev.speed)
      .Add("EV - Special Attack", pkm->ev.special_attack)
      .Add("EV - Special Defense", pkm->ev.special_defense)
      .AddSeparator();

  menu.Add("IV - HP", &pkm->iv_flags, 0, 5)
      .Add("IV - Attack", &pkm->iv_flags, 5, 5)
      .Add("IV - Defense", &pkm->iv_flags, 10, 5)
      .Add("IV - Speed", &pkm->iv_flags, 15, 5)
      .Add("IV - Special Attack", &pkm->iv_flags, 20, 5)
      .Add("IV - Special Defense", &pkm->iv_flags, 25, 5)
      .AddSeparator();

  menu.Add("Contest - Cool", pkm->contest.cool)
      .Add("Contest - Beautiful", pkm->contest.beautiful)
      .Add("Contest - Cute", pkm->contest.cute)
      .Add("Contest - Smart", pkm->contest.smart)
      .Add("Contest - Tough", pkm->contest.tough)
      .Add("Contest - Sheen", pkm->contest.sheen);
}

void LoadSaveDataTeamMenu(PluginMenu& menu, void* args) {
  static u8 slot_idx = 0;
  auto& data = savedata::PokemonTeam::GetInstance();

  menu.Add("Count", data.count)
      .WithBounds(0, 6)
      .Add("Slot Index", slot_idx)
      .WithBounds(0, 5)
      .WithRefresh()
      .AddSeparator();

  LoadSaveDataPokemonMenu(menu, data.pokemons[slot_idx]->core);
}

void LoadSaveDataBattleBoxMenu(PluginMenu& menu, void* args) {
  static u8 slot_idx = 0;
  auto& data = savedata::BattleBox::GetInstance();

  menu.Add("Slot Index", slot_idx)
      .WithBounds(0, 5)
      .WithRefresh()
      .AddSeparator();

  LoadSaveDataPokemonMenu(menu, &data.pokemons[slot_idx]);
}

void LoadSaveDataPokemonBoxMenu(PluginMenu& menu, void* args) {
  static u8 box_idx = 0;
  static u8 slot_idx = 0;

  auto& data = savedata::PokemonBox::GetInstance();
  menu.Add("Box Index", box_idx)
      .WithBounds(0, 30)
      .WithRefresh()
      .Add("Slot Index", slot_idx)
      .WithBounds(0, 29)
      .WithRefresh()
      .AddSeparator();

  LoadSaveDataPokemonMenu(
      menu, &data.boxes[box_idx].pokemons[slot_idx]);
}

void LoadSaveDataBoxesMetadataMenu(PluginMenu& menu, void* args) {
  static u8 index = 0;
  auto& data = savedata::BoxManager::GetInstance();

  menu.Add("Box Index", index)
      .WithBounds(0, 30)
      .WithRefresh()
      .AddSeparator()
      .Add("Title", data.titles[index], 17)
      .Add("Wallpaper", data.wallpapers[index])
      .WithBounds(0, 23)
      .Add("Unlocked Boxes Count", data.unlocked_count)
      .Add("Active Box Index", data.active_box_index)
      .WithBounds(0, 30)
      .Add("Special Wallpapers Unlocked", &data.flags, 0, 7)
      .Add("Event Box Open", &data.flags, 7, 1);
}

void LoadSaveDataBagItemsMenu(PluginMenu& menu, void* args) {
  static u32 pocket_id = 0;
  static u32 slot_idx = 0;

  static const c8* pocket_names[] = {
      "Items", "Key Items", "TMs & HMs",
      "Medicine", "Berries"
  };

  menu.Add("Pocket Select", pocket_id)
      .WithArray(pocket_names, 5)
      .WithRefresh();

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

  menu.Add("Slot Select", slot_idx)
      .WithBounds(0, max_slots - 1)
      .WithRefresh()
      .AddSeparator();

  menu.AddItem("Item", target_pocket[slot_idx].id);

  menu.Add("Quantity", target_pocket[slot_idx].count)
      .WithBounds(0, savedata::ItemManager::kMaxItemCount);
}

#include "game/savedata/records.inc"

void LoadSaveDataRecordsMenu(PluginMenu& menu, void* args) {
  static u32 record_0_idx = 0;
  static u32 record_1_idx = 0;

  auto& data = savedata::RecordManager::GetInstance();

  menu.Add("Is Disabled", data.is_disabled)
      .AddSeparator()
      .Add("Record 0", record_0_idx)
      .WithArray(RECORDS_0, SIZE(RECORDS_0))
      .WithRefresh()
      .Add("Value", data.records_0[record_0_idx])
      .AddSeparator()
      .Add("Record 1", record_1_idx)
      .WithArray(RECORDS_1, SIZE(RECORDS_1))
      .WithRefresh()
      .Add("Value", data.records_1[record_1_idx]);
}

void LoadSaveDataMiscellaneousMenu(PluginMenu& menu, void* args) {
  auto& data = savedata::Misc::GetInstance();

  menu.Add("Money", data.money)
      .Add("Battle Points", data.battle_points)
      .AddSeparator()
      .Add("Badge 1", &data.badges, 0, 1)
      .Add("Badge 2", &data.badges, 1, 1)
      .Add("Badge 3", &data.badges, 2, 1)
      .Add("Badge 4", &data.badges, 3, 1)
      .Add("Badge 5", &data.badges, 4, 1)
      .Add("Badge 6", &data.badges, 5, 1)
      .Add("Badge 7", &data.badges, 6, 1)
      .Add("Badge 8", &data.badges, 7, 1)
      .AddSeparator()
      .Add("Rival Nickname", data.rival_nickname,
           savedata::Misc::kNicknameLength)
      .Add("Unlock Pokémon League Wallpapers", &data.flags, 0, 1)
      .Add("Keyboard Layout", &data.flags, 2, 1)
      .Add("Exp. Share Enabled", &data.flags, 3, 1)
      .Add("PSS Tutorial Seen", &data.flags, 5, 1)
      .Add("Pokemon-Amie Tutorial Seen", &data.flags, 6, 1)
      .Add("Super Training Tutorial Seen", &data.flags, 7, 1)
      .Add("Vs. Recorder Tutorial Seen", &data.flags, 9, 1)
      .Add("Skip Long Sky Trip Animation", &data.flags, 11, 1)
      .Add("TV Navi Tutorial Seen", &data.flags, 14, 1);
}

void LoadSaveDataTrainerStatusMenu(PluginMenu& menu, void* args) {
  auto& data = savedata::TrainerStatus::GetInstance();

  menu.Add("Player Name", data.name, savedata::TrainerStatus::kPlayerNameLen)
      .Add("Nickname", data.nickname, savedata::TrainerStatus::kPlayerNameLen)
      .Add("Visible Trainer ID (TID)", data.trainer_id)
      .Add("Secret Trainer ID (SID)", data.secret_id)
      .Add("Gender (0:M, 1:F)", data.gender)
      .AddSeparator()

      .Add("Game Version", data.game_version)
      .Add("Language", data.language)
      .Add("Mega Ring Obtained", &data.mega_flags, 0, 1)
      .Add("Mega Rayquaza Evolution Unlock", &data.mega_flags, 1, 1)
      .AddSeparator()

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
      .AddSeparator()

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

void LoadSaveDataOverworldMenuMenu(PluginMenu& menu, void* args) {
  auto& data = savedata::OverworldMenu::GetInstance();

  menu.Add("Is Pokemon List Visible", &data.flags, 0, 1)
      .Add("Pokemon List Position", &data.flags, 6, 3)
      .AddSeparator()

      .Add("Is Pokedex Visible", &data.flags, 1, 1)
      .Add("Pokedex Position", &data.flags, 9, 3)
      .AddSeparator()

      .Add("Is Bag Visible", &data.flags, 2, 1)
      .Add("Bag Position", &data.flags, 12, 3)
      .AddSeparator()

      .Add("Is Trainer Card Visible", &data.flags, 3, 1)
      .Add("Trainer Card Position", &data.flags, 15, 3)
      .AddSeparator()

      .Add("Is Save Visible", &data.flags, 4, 1)
      .Add("Save Position", &data.flags, 18, 3)
      .AddSeparator()

      .Add("Is Options Visible", &data.flags, 5, 1)
      .Add("Options Position", &data.flags, 21, 3);
}

void LoadSaveDataMinigameMenu(PluginMenu& menu, void* args) {
  static u32 choice = 0;
  static u32 puzzle_idx = 0;

  static const char* DIFFICULTIES[] = {
      "Easy", "Normal", "Hard", "Unlimited"
  };

  static const char* RATINGS[] = {
      "None", "1 Star", "2 Stars", "3 Stars", "4 Stars", "4.5 Stars", "5 Stars"
  };

  auto& data = savedata::Minigame::GetInstance();

  menu.Add("Difficulty Choice", choice)
      .WithArray(DIFFICULTIES, 4)
      .WithRefresh();

  menu.AddSeparator();

  menu.Add("Berry Picker Score", data.berry_picker_high_scores[choice])
      .WithBounds(0, 999)
      .Add("Berry Picker Rating", data.berry_picker_best_ratings[choice])
      .WithArray(RATINGS, SIZE(RATINGS))
      .AddSeparator();

  menu.Add("Head It Score", data.head_it_high_scores[choice])
      .WithBounds(0, 9999)
      .Add("Head It Rating", data.head_it_best_ratings[choice])
      .WithArray(RATINGS, SIZE(RATINGS))
      .AddSeparator();

  menu.Add("Tile Puzzle Index", puzzle_idx)
      .WithBounds(0, 4)
      .WithRefresh();

  auto& puzzle = data.tile_puzzle_scores[choice];

  menu.Add("Puzzle Score", puzzle.total_score[puzzle_idx])
      .Add("Puzzle Time (s)", puzzle.time[puzzle_idx])
      .Add("Puzzle Moves", puzzle.moves[puzzle_idx])
      .Add("Puzzle Swaps", puzzle.swaps[puzzle_idx])
      .Add("Puzzle Rating", data.tile_puzzle_best_ratings[choice])
      .WithArray(RATINGS, SIZE(RATINGS));
}

void LoadSaveDataPokemonAmieMenu(PluginMenu& menu, void* args) {
  static u8 puff_idx = 0;
  auto& data = savedata::PokemonAmie::GetInstance();

  menu.Add("Select Slot", puff_idx)
      .WithBounds(0, savedata::PokemonAmie::kMaxPokePuffs - 1)
      .WithRefresh();

  menu.Add("Poké Puff ID", data.poke_puffs[puff_idx])
      .WithBounds(0, savedata::PokemonAmie::kMaxPokePuffId);

  menu.Add("Last Opened (Days)", data.last_opened_timestamp);
}

void LoadSaveDataBagMetadataMenu(PluginMenu& menu, void* args) {
  static u8 pocket_idx = 0;
  static u8 register_idx = 0;
  static u8 history_idx = 0;
  static const c8* pocket_type[savedata::BagManager::kMaxPockets] = {
      "Items", "Medecine", "TMs & HMs",
      "Berries", "Key Items"
  };

  auto& data = savedata::BagManager::GetInstance();

  menu.Add("Pocket Select", pocket_idx)
      .WithBounds(0, savedata::BagManager::kMaxPockets - 1)
      .WithRefresh()
      .AddSeparator()
      .Add("Pocket Type", data.pocket_order[pocket_idx])
      .WithArray(pocket_type, savedata::BagManager::kMaxPockets)
      .Add("Shortcut Select", register_idx)
      .WithBounds(0, savedata::BagManager::kMaxRegisteredItems - 1)
      .WithRefresh()
      .AddItem("Registered Item", data.registered_items[register_idx])
      .Add("History Select", history_idx)
      .WithBounds(0, savedata::BagManager::kMaxUsageHistory - 1)
      .WithRefresh()
      .AddItem("Last Item Used", data.last_items_used[history_idx]);
}

#include "game/savedata/pokedex_form.inc"

void LoadSaveDataPokedexMenu(PluginMenu& menu, void* args) {
  static u16 species = 1;
  static u8 form = 0;
  static u16 prev_species = species;
  if (species != prev_species) {
    form = 0;
    prev_species = species;
  }

  u32 idx, bit_pos, array_idx;
  auto& data = savedata::Pokedex::GetInstance();

  menu.Add("Spinda Pattern", data.spinda_pattern)
      .AddSeparator();

  menu.AddSpecies("Species", species)
      .WithBounds(1, 721)
      .WithRefresh()
      .AddSeparator();

  idx = data.GetFormIndex(species);
  if (idx != -1) {
    s32 table_index, form_max;
    savedata::Pokedex::GetTableIndexAndFormMax(table_index, form_max, species);
    if (form >= form_max) form = 0;

    idx += form;
    bit_pos = idx & 31;
    array_idx = idx >> 5;

    u32* normal_form_seen_flags = (u32*)data.form_seen_flags[0];
    u32* shiny_form_seen_flags = (u32*)data.form_seen_flags[1];

    menu.Add("Form", form)
        .WithArray(FORMS[table_index], form_max)
        .WithRefresh()
        .Add("Form Seen: Normal",
             &normal_form_seen_flags[array_idx], bit_pos,
             1)
        .Add("Form Seen: Shiny", &shiny_form_seen_flags[array_idx], bit_pos, 1);

    u32* normal_displayed_form_flags = (u32*)data.displayed_form_flags[0];
    u32* shiny_displayed_form_flags = (u32*)data.displayed_form_flags[1];

    menu.Add("Display Form: Normal", &normal_displayed_form_flags[array_idx],
             bit_pos, 1)
        .Add("Display Form: Shiny", &shiny_displayed_form_flags[array_idx],
             bit_pos, 1)
        .AddSeparator();
  }
  idx = species - 1;
  bit_pos = idx & 31;
  array_idx = idx >> 5;

  menu.Add("Captured", &data.captured_flags[array_idx], bit_pos, 1)
      .Add("Times Encountered", data.seen_count[species])
      .WithBounds(0, 999)
      .AddSeparator()
      .Add("Seen: Male", &data.gender_seen_flags[0][array_idx], bit_pos, 1)
      .Add("Seen: Female", &data.gender_seen_flags[1][array_idx], bit_pos, 1)
      .Add("Seen: Shiny Male", &data.gender_seen_flags[2][array_idx], bit_pos,
           1)
      .Add("Seen: Shiny Female", &data.gender_seen_flags[3][array_idx], bit_pos,
           1)
      .AddSeparator()
      .Add("Display: Male", &data.displayed_gender_flags[0][array_idx],
           bit_pos, 1)
      .Add("Display: Female", &data.displayed_gender_flags[1][array_idx],
           bit_pos, 1)
      .Add("Display: Shiny Male",
           &data.displayed_gender_flags[2][array_idx], bit_pos, 1)
      .Add("Display: Shiny Female",
           &data.displayed_gender_flags[3][array_idx], bit_pos, 1);
}

#include "game/savedata/opower.inc"

void LoadSaveDataOPowerMenu(PluginMenu& menu, void* args) {
  static u32 learned_opower_idx = 0;
  static u32 field_opower_idx = 0;
  static u32 battle_opower_idx = 0;

  auto& man = savedata::OPowerManager::GetInstance();

  menu.Add("OPower Points", man.power_points)
      .Add("Learned OPower Index", learned_opower_idx)
      .WithArray(OPOWERS, SIZE(OPOWERS))
      .WithRefresh()
      .Add("Learned OPower Value", man.learned_powers[learned_opower_idx])
      .AddSeparator()
      .Add("Field OPower Index", field_opower_idx)
      .WithArray(FIELD_OPOWERS, SIZE(FIELD_OPOWERS))
      .WithRefresh()
      .Add("Field Lv. 1 Uses", man.field_power_level_1_uses[field_opower_idx])
      .Add("Field Lv. 2 Uses", man.field_power_level_2_uses[field_opower_idx])
      .AddSeparator()
      .Add("Battle OPower Index", battle_opower_idx)
      .WithArray(BATTLE_OPOWERS, SIZE(BATTLE_OPOWERS))
      .WithRefresh()
      .Add("Battle Lv. 1 Uses",
           man.battle_power_level_1_uses[battle_opower_idx])
      .Add("Battle Lv. 2 Uses",
           man.battle_power_level_2_uses[battle_opower_idx]);
}

void LoadSaveDataPlayTimeMenu(PluginMenu& menu, void* args) {
  auto& data = savedata::PlayTime::GetInstance();

  menu.Add("Hour", data.hour)
      .Add("Minute", data.minute)
      .Add("Second", data.second);
}

void OnUpdateLanguage(void*) {
  auto& settings = savedata::Settings::GetInstance();
  Core::GetInstance().GetLanguageId() = settings.language_id;
  WRITE(vu32, ADDRESS_LANGUAGE_ID, settings.language_id);
}

void LoadSaveDataSettingsMenu(PluginMenu& menu, void* args) {
  static const c8* TEXT_SPEED[] = {"Slow", "Normal", "Fast", "Instant"};
  static const c8* TOGGLE_OFF_ON[] = {"Off", "On"};
  static const c8* BATTLE_STYLE[] = {"Shift", "Set"};
  static const c8* BUTTON_MODE[] = {"Normal", "L=A", "LR Disabled"};
  static const c8* LANGUAGES[] = {
      "None", "Japanese", "English",
      "French", "Italian", "German",
      "---", "Spanish", "Korean"
  };
  static const c8* BATTLE_BACKGROUNDS[] = {
      "Default", "Red", "Blue", "Pikachu", "Starters",
      "Eevee", "Monochrome", "Stickers", "Tatami", "Floral Pattern",
      "Elegant", "Tall Grass", "Poke Ball", "Cockpit", "Carbon"
  };

  auto& settings = savedata::Settings::GetInstance();

  menu.Add("Text Speed", &settings.core, 0, 2)
      .WithArray(TEXT_SPEED, SIZE(TEXT_SPEED))

      .Add("Show Battle Animations", &settings.core, 2, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))

      .Add("Battle Style", &settings.core, 3, 1)
      .WithArray(BATTLE_STYLE, SIZE(BATTLE_STYLE))

      .Add("Language ID (Press A)", &settings.core, 4, 4)
      .WithCallback(OnUpdateLanguage)
      .WithArray(LANGUAGES, SIZE(LANGUAGES))

      .Add("Battle Background", &settings.core, 8, 5)
      .WithArray(BATTLE_BACKGROUNDS, SIZE(BATTLE_BACKGROUNDS))

      .Add("Button Config Mode", &settings.core, 13, 2)
      .WithArray(BUTTON_MODE, SIZE(BUTTON_MODE))

      .Add("Prompt Save Before Net", &settings.core, 15, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))

      .Add("Enable SpotPass", &settings.core, 16, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))

      .Add("Enable PSS", &settings.core, 17, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON));
}

void LoadSaveDataEncounterMenu(PluginMenu& menu, void* args) {
  auto& data = savedata::Encounter::GetInstance();

  menu.AddItem("Spray Type", data.spray_id)
      .Add("Spray Count", data.spray_count);
}

#include "game/savedata/pss.inc"

void LoadSaveDataPssProfileMenu(PluginMenu& menu, void* args) {
  auto& profile = *(savedata::PssProfilePayload*)args;

  menu.Add("Name", profile.name, 13)
      .Add("Shout-out Message", profile.shoutout_message, 17)
      .Add("Principal Id", profile.principal_id)
      .Add("Local Friend Code", profile.local_friend_code)

      .Add("Icon (For X/Y)", &profile.flags, 8, 8)
      .WithArray(ICONS, SIZE(ICONS))
      .Add("Icon (For OR/AS)", &profile.flags2, 13, 8)
      .WithArray(ICONS, SIZE(ICONS))
      .Add("Gender", &profile.flags, 16, 4)
      .Add("Has Accepted EULA", &profile.flags, 20, 1)
      .Add("Has Promotion Video", &profile.flags, 21, 1)
      .Add("Has Pokemon In Party", &profile.flags, 22, 1)
      .Add("Has Hall Of Fame Completed", &profile.flags, 23, 1)
      .Add("Rejects Friend Requests", &profile.flags, 24, 1)
      .Add("Rejects Acquaintance Requests", &profile.flags, 25, 1)
      .Add("Rejects Passerby Requests", &profile.flags, 26, 1)
      .Add("Rejects Voice Chat", &profile.flags, 27, 1)
      .Add("Parental: Photo Sharing Disabled", &profile.flags, 28, 1)
      .Add("Parental: Internet Disabled", &profile.flags, 29, 1)
      .Add("Parental: Friend Reg. Closed", &profile.flags, 30, 1)
      .Add("Parental: Streetpass Disabled", &profile.flags, 31, 1)

      .Add("Geographic Region Id", profile.geographic_region_id)
      .Add("Birth Month", profile.birth_month)
      .Add("Birth Day", profile.birth_day)
      .Add("Game Version", profile.game_version)
      .Add("Console Region", profile.console_region)

      .Add("Rejects Promo Video Requests", &profile.flags2, 0, 1)
      .Add("Has Shout-Out Message", &profile.flags2, 1, 1)
      .Add("Meets Trade Conditions", &profile.flags2, 2, 1)
      .Add("Rejects Lower Version Battles", &profile.flags2, 3, 1)
      .Add("Rejects Lower Version PR Videos", &profile.flags2, 4, 1);
}

void LoadSaveDataPssGroupMenu(PluginMenu& menu, void* args) {
  static u32 choice = 0;
  auto& grp = *(savedata::PssGroup*)args;
  menu.Add("Profile Index", choice).AddSeparator();
  LoadSaveDataPssProfileMenu(menu, &grp.user_data[choice].datagram.profile);
}

void LoadSaveDataMenu(PluginMenu& menu, void* args) {
  auto& sv = savedata::SaveData::GetInstance();

  menu.Add("Team", LoadSaveDataTeamMenu)
      .Add("PC", LoadSaveDataPokemonBoxMenu)
      .Add("Battle Box", LoadSaveDataBattleBoxMenu)
      .AddSeparator()

      .Add("Bag Items", LoadSaveDataBagItemsMenu)
      .Add("Miscellaneous", LoadSaveDataMiscellaneousMenu)
      .Add("Trainer Status", LoadSaveDataTrainerStatusMenu)
      .Add("Pokedex", LoadSaveDataPokedexMenu)
      .Add("Play Time", LoadSaveDataPlayTimeMenu)
      .AddSeparator()

      .Add("O-Power", LoadSaveDataOPowerMenu)
      .Add("Encounter", LoadSaveDataEncounterMenu)
      .Add("Settings", LoadSaveDataSettingsMenu)
      .Add("Overworld Menu", LoadSaveDataOverworldMenuMenu)
      .AddSeparator()

      .Add("Pokemon-Amie", LoadSaveDataPokemonAmieMenu)
      .Add("Minigame", LoadSaveDataMinigameMenu)
      .Add("Records", LoadSaveDataRecordsMenu)
      .Add("Bag Metadata", LoadSaveDataBagMetadataMenu)
      .Add("Boxes Metadata", LoadSaveDataBoxesMetadataMenu)
      .AddSeparator()

      .Add("PSS - Favourites", LoadSaveDataPssGroupMenu,
           &sv.GetPssFavouriteGroup())
      .Add("PSS - Friends", LoadSaveDataPssGroupMenu, &sv.GetPssFriendGroup())
      .Add("PSS - Acquaintances", LoadSaveDataPssGroupMenu,
           &sv.GetPssAcquaintanceGroup());
}
}