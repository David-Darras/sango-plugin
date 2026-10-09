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
 * @file page_pokemon.cc
 * @brief The menu pages of the Pokemon family.
 */

#include "overworld/patch/gift_pokemon.h"
#include "pokemon/patch/alolan_forms.h"
#include "pokemon/patch/species_table.h"
#include <cstring>
#include "pokemon/native/data_accessor.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/move_data.h"
#include "pokemon/native/species_data.h"
#include "pokemon/patch/custom_shop.h"
#include "pokemon/native/utils.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/pokemon_box.h"
#include "ui/log_application.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/trade.h"
#include "pokemon/patch/shiny.h"
#include "overworld/patch/whos_that_pokemon.h"
#include "pokemon/patch/item_customizer.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/patch/app_status.h"
#include "pokemon/data/za_mega.inc"

namespace ui {
void LoadShinyPage(MainApplication& app, void* args) {
  static const c8* SHINY_RATES[] = {"Off", "1/1", "1/2", "1/4", "1/8", "1/16",
                                    "1/32", "1/64", "1/128", "1/256", "1/512",
                                    "1/1024", "1/2048", "1/4096", "1/8192",
                                    "1/16384", "1/32768", "1/65536", "1/131072",
                                    "1/262144", "1/524288", "1/1048576"};
  static_assert(
      SIZE(SHINY_RATES) == static_cast<u32>(pokemon::ShinyRate::kCount),
      "one label per ShinyRate");

  auto& shiny = pokemon::Shiny::GetInstance();

  app.Add("Shiny Rate", shiny.rate)
     .WithArray(SHINY_RATES, SIZE(SHINY_RATES))
     .WithBounds(0, SIZE(SHINY_RATES) - 1);
}

static void DefaultMoves(SpeciesId species, FormId data_form,
                         MoveId (&learnt)[4]) {
  auto& pool = pokemon::Movepool::GetInstance(species, data_form);
  for (u32 k = 0; k < 4; k++) learnt[k] = MoveId::kNone;
  u32 count = 0;
  for (u32 k = 0; k < pool.count; k++) {
    if (pool.entry[k].level > 100) break;
    learnt[count % 4] = pool.entry[k].move;
    count++;
  }
}

static bool PutInBox(u32 slot, SpeciesId species, FormId form,
                     FormId data_form, ItemId item = ItemId::kNone) {
  const u32 box = slot / savedata::PokemonBox::kMaxSlotsPerBox;
  const u32 index = slot % savedata::PokemonBox::kMaxSlotsPerBox;
  if (box >= savedata::PokemonBox::kMaxBoxes) return false;

  auto& team = savedata::PokemonTeam::GetInstance();
  auto& boxes = savedata::PokemonBox::GetInstance();
  static pokemon::CoreData scratch;
  pokemon::DataAccessor accessor;
  std::memcpy(&scratch, team.pokemons[0]->core, sizeof(scratch));
  accessor.Initialize(&scratch, nullptr);
  accessor.Decrypt();
  scratch.species = species;
  scratch.form = form;
  if (item != ItemId::kNone) scratch.item = item;
  scratch.is_illegal_egg = false;
  scratch.is_egg = false;
  scratch.SetShiny(false);
  scratch.ability = pokemon::SpeciesData::GetInstance(species, form).ability[0];
  scratch.experience =
      pokemon::Utils::GetExperienceFromLevel(species, data_form, 100);
  MoveId learnt[4];
  DefaultMoves(species, data_form, learnt);
  scratch.SetMoves(learnt[0], learnt[1], learnt[2], learnt[3]);
  for (u32 k = 0; k < 4; k++) scratch.pp[k] = 20;
  scratch.ResetNickname();
  accessor.Encrypt();
  std::memcpy(&boxes.boxes[box].pokemons[index], &scratch, sizeof(scratch));
  return true;
}

static void FillBoxesWithAll(void*) {
  if (savedata::PokemonTeam::GetInstance().count == 0) return;
  u32 slot = 0;
  for (u32 i = 0; i < pokemon::SpeciesTable::kGen7Count; i++) {
    const auto species =
        static_cast<SpeciesId>(pokemon::SpeciesTable::kFirstGen7Species + i);
    if (!PutInBox(slot++, species, FormId::kNormal, FormId::kNormal)) return;
  }
  for (u32 i = pokemon::SpeciesTable::kGen8Skipped;
       i < pokemon::SpeciesTable::kGen8Count; i++) {
    const auto species = static_cast<SpeciesId>(
      pokemon::SpeciesTable::kFirstGen8Species +
      (i - pokemon::SpeciesTable::kGen8Skipped));
    if (pokemon::SpeciesData::GetInstance(species).base_hp == 0) continue;
    if (!PutInBox(slot++, species, FormId::kNormal, FormId::kNormal)) return;
  }
  const u16 last = static_cast<u16>(pokemon::SpeciesTable::kFirstGen8Species +
                                    pokemon::SpeciesTable::kGen8Count);
  for (u16 i = 1; i < last; i++) {
    for (u32 rank = 0; rank < pokemon::AlolanForms::FormCount(i); rank++) {
      if (!PutInBox(slot++, static_cast<SpeciesId>(i),
                    static_cast<FormId>(pokemon::AlolanForms::GetForm(i, rank)),
                    FormId::kNormal))
        return;
    }
  }
  for (u32 i = 0; i < SIZE(kZaMegas); i++) {
    if (!PutInBox(slot++, kZaMegas[i].species, FormId::kNormal, FormId::kNormal,
                  kZaMegas[i].item))
      return;
  }
}

static void SetTeamSlot(u32 slot, SpeciesId species, ItemId item) {
  savedata::PokemonParam* pokemon =
      savedata::PokemonTeam::GetInstance().pokemons[slot];
  pokemon->accessor->Decrypt();
  pokemon::CoreData* core = pokemon->core;
  core->species = species;
  core->form = FormId::kNormal;
  core->item = item;
  core->is_egg = 0;
  core->is_illegal_egg = 0;
  core->SetShiny(false);
  core->SetLevel(100);
  core->ability =
      pokemon::SpeciesData::GetInstance(species, FormId::kNormal).ability[0];
  MoveId learnt[4];
  DefaultMoves(species, FormId::kNormal, learnt);
  core->SetMoves(learnt[0], learnt[1], learnt[2], learnt[3]);
  for (u32 k = 0; k < 4; k++) {
    core->pp[k] = learnt[k] == MoveId::kNone
                    ? 0
                    : pokemon::MoveData::GetInstance(learnt[k]).base_pp;
    core->pp_up_count[k] = 0;
  }
  core->ResetNickname();
  pokemon->accessor->Encrypt();
  pokemon->UpdateRuntimeData();
}

static u8 za_index = 0;

static void GiveZaMegaToTeam(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return;
  const ZaMega& mega = kZaMegas[za_index];
  if (team.count >= savedata::PokemonTeam::kMaxSlots ||
      !pokemon::CustomShop::GiveMega(mega.species, mega.item)) {
    SetTeamSlot(team.count - 1, mega.species, mega.item);
  }
  team.HealAllPokemons();
}

static void FillTeamWithZaMegas(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return;
  for (u32 i = 0; i < savedata::PokemonTeam::kMaxSlots; i++) {
    const ZaMega& mega = kZaMegas[(za_index + i) % SIZE(kZaMegas)];
    if (i < team.count) {
      SetTeamSlot(i, mega.species, mega.item);
    } else if (!pokemon::CustomShop::GiveMega(mega.species, mega.item)) {
      break;
    }
  }
  team.HealAllPokemons();
}

static void FillBoxesWithZaMegas(void*) {
  if (savedata::PokemonTeam::GetInstance().count == 0) return;
  for (u32 i = 0; i < SIZE(kZaMegas); i++) {
    if (!PutInBox(i, kZaMegas[i].species, FormId::kNormal, FormId::kNormal,
                  kZaMegas[i].item))
      break;
  }
}

static void FillBoxesWithZaMegaForms(void*) {
  if (savedata::PokemonTeam::GetInstance().count == 0) return;
  for (u32 i = 0; i < SIZE(kZaMegas); i++) {
    if (!PutInBox(i, kZaMegas[i].species, static_cast<FormId>(kZaMegas[i].form),
                  FormId::kNormal, kZaMegas[i].item))
      break;
  }
}

// The Mega Evolutions of Pokemon Legends: Z-A: give them to the team or put
// them in the boxes.
static void LoadZaMegaPage(MainApplication& app, void* args) {
  static const c8* names[SIZE(kZaMegas)] = {};
  if (names[0] == nullptr) {
    for (u32 i = 0; i < SIZE(kZaMegas); i++) names[i] = kZaMegas[i].name;
  }

  app.AddSection("One Pokemon")
     .Add("Mega Evolution", za_index)
     .WithArray(names, SIZE(names))
     .Add("Give To The Team", GiveZaMegaToTeam)
     .WithDescription("Gives the Pokemon and its item. When the team is "
                      "full, it replaces the last Pokemon.")
     .AddSection("All The Pokemon")
     .Add("Fill The Team", FillTeamWithZaMegas)
     .WithDescription("Replaces the team with 6 Pokemon of the list, from "
                      "the selected one.")
     .Add("Fill The Boxes", FillBoxesWithZaMegas)
     .WithDescription("Puts all the Pokemon of the list in the boxes. It "
                      "replaces the Pokemon in the first boxes!")
     .Add("Fill The Boxes (Mega Forms)", FillBoxesWithZaMegaForms)
     .WithDescription("The same, with the Mega forms. It replaces the "
                      "Pokemon in the first boxes!");
}

void LoadPokemonPage(MainApplication& app, void* args) {
  auto& who = overworld::WhosThatPokemon::GetInstance();

  app.AddSection("Shiny");
  LoadShinyPage(app, args);
  app.WithDescription("The chance that a new Pokemon is shiny. Off: the "
                      "normal rate of the game.")
     .AddSection("Randomizers")
     .Add("Randomize Gift Pokemon",
          overworld::GiftPokemon::GetInstance().randomize_species)
     .WithDescription("The Pokemon that characters give are random.")
     .Add("Randomize Static Encounters",
          overworld::StaticRandomizer::GetInstance().randomize_species)
     .WithDescription("The legendary Pokemon and the other fixed encounters "
                      "are random.")
     .Add("Randomize Trades",
          overworld::Trade::GetInstance().randomize_species)
     .WithDescription("The Pokemon of the in-game trades are random.")
     .AddSection("Rules")
     .Add("No EV Limit", pokemon::ItemCustomizer::GetInstance().remove_limit)
     .WithDescription("The items that raise the EVs (vitamins, wings) work "
                      "up to 255 for each stat.")
     .Add("Restricted Summary Editor", AppStatus::GetInstance().is_restricted)
     .WithDescription("On: the summary editor changes only the nature, the "
                      "ability and the form.")
     .Add("Pokemon Shop", CheatCodeId::kPokemonShop)
     .WithDescription("The normal Poke Marts sell Pokemon.")
     .AddSection("Data")
     .Add("Species Data", LoadSpeciesDataPage)
     .WithDescription("Base stats, types, abilities and TMs of each species.")
     .Add("Move Data", LoadMoveDataPage)
     .WithDescription("Type, power, accuracy and effects of each move.")
     .Add("TMs and HMs", LoadTechnicalMachinePage)
     .WithDescription("The move of each TM and HM.")
     .AddSection("Tools")
     .Add("Z-A Mega Evolutions", LoadZaMegaPage)
     .WithDescription("Gives the Mega Evolutions of Pokemon Legends: Z-A.")
#ifdef GAME_ORAS
     .Add("Fill Boxes With All New (Lv. 100)", FillBoxesWithAll)
     .WithDescription("Puts each new species and form in the boxes. It "
                      "replaces the Pokemon in the boxes!")
#endif
     .Add("Model Loader (Unstable)", LoadModelLoaderPage)
     .WithDescription("Shows a model of a character or a Pokemon near the "
                      "player.")
     .AddSection("Who's That Pokemon?")
     .Add("Reward Level", who.reward_level)
     .WithBounds(1, 100)
     .WithDescription("The level of the Pokemon that the player wins.")
     .Add("Highest Species", who.max_species)
     .WithBounds(1, static_cast<u32>(SpeciesId::kCount) - 1)
     .WithDescription("The questions use the species from 1 to this "
                      "number.");
}
} // namespace ui