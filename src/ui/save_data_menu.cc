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

#include "menu/plugin_menu.h"
#include "savedata/battle_box.h"
#include "savedata/box_manager.h"
#include "savedata/item_manager.h"
#include "savedata/pokemon_box.h"
#include "savedata/pokemon_core_data.h"
#include "savedata/pokemon_data_accessor.h"
#include "savedata/pokemon_team.h"
#include "savedata/pokemon_utils.h"

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

void LoadSaveDataPokemonMenu(menu::PluginMenu& menu, void* args) {
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

  menu.AddMove("Move 1", pkm->moves[0])
      .AddMove("Move 2", pkm->moves[1])
      .AddMove("Move 3", pkm->moves[2])
      .AddMove("Move 4", pkm->moves[3]);

  menu.Add("PP 1", pkm->pp[0])
      .Add("PP 2", pkm->pp[1])
      .Add("PP 3", pkm->pp[2])
      .Add("PP 4", pkm->pp[3]);

  menu.Add("EV - HP", pkm->ev.hp)
      .Add("EV - Attack", pkm->ev.attack)
      .Add("EV - Defense", pkm->ev.defense)
      .Add("EV - Speed", pkm->ev.speed)
      .Add("EV - Special Attack", pkm->ev.special_attack)
      .Add("EV - Special Defense", pkm->ev.special_defense);

  menu.Add("IV - HP", &pkm->iv_flags, 0, 5)
      .Add("IV - Attack", &pkm->iv_flags, 5, 5)
      .Add("IV - Defense", &pkm->iv_flags, 10, 5)
      .Add("IV - Speed", &pkm->iv_flags, 15, 5)
      .Add("IV - Special Attack", &pkm->iv_flags, 20, 5)
      .Add("IV - Special Defense", &pkm->iv_flags, 25, 5);

  menu.Add("Contest - Cool", pkm->contest.cool)
      .Add("Contest - Beautiful", pkm->contest.beautiful)
      .Add("Contest - Cute", pkm->contest.cute)
      .Add("Contest - Smart", pkm->contest.smart)
      .Add("Contest - Tough", pkm->contest.tough)
      .Add("Contest - Sheen", pkm->contest.sheen);
}

void LoadSaveDataTeamMenu(menu::PluginMenu& menu, void* args) {
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

void LoadSaveDataBattleBoxMenu(menu::PluginMenu& menu, void* args) {
  static u8 slot_idx = 0;
  auto& data = savedata::BattleBox::GetInstance();

  menu.Add("Slot Index", slot_idx)
      .WithBounds(0, 5)
      .WithRefresh()
      .AddSeparator();

  LoadSaveDataPokemonMenu(menu, &data.pokemons[slot_idx]);
}

void LoadSaveDataPokemonBoxMenu(menu::PluginMenu& menu, void* args) {
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

void LoadSaveDataBoxesMetadataMenu(menu::PluginMenu& menu, void* args) {
  static u8 index = 0;
  auto& data = savedata::BoxManager::GetInstance();

  menu.Add("Box Index", index)
      .WithBounds(0, 30)
      .WithRefresh()
      .Add("Title", data.titles[index], 17)
      .Add("Wallpaper", data.wallpapers[index])
      .WithBounds(0, 23)
      .Add("Unlocked Boxes Count", data.unlocked_count)
      .Add("Active Box Index", data.active_box_index)
      .WithBounds(0, 30)
      .Add("Special Wallpapers Unlocked", &data.flags, 0, 7)
      .Add("Event Box Open", &data.flags, 7, 1);
}

void LoadSaveDataBagItemsMenu(menu::PluginMenu& menu, void* args) {
  static u32 pocket_id = 0;
  static u32 slot_idx = 0;

  static const c8* pocket_names[] = {"Items", "Key Items", "TMs & HMs",
                                     "Medicine", "Berries"};

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

void LoadSaveDataMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Team", LoadSaveDataTeamMenu)
      .Add("Battle Box", LoadSaveDataBattleBoxMenu)
      .Add("PC", LoadSaveDataPokemonBoxMenu)
      .AddSeparator()
      .Add("Bag Items", LoadSaveDataBagItemsMenu)
      .Add("Boxes Metadata", LoadSaveDataBoxesMetadataMenu);
}
}