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

#include "overworld/patch/gift_pokemon.h"
#include "pokemon/patch/alolan_forms.h"
#include "pokemon/patch/species_table.h"
#include <cstring>
#include "pokemon/native/data_accessor.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/utils.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/pokemon_box.h"
#include "ui/log_application.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/trade.h"
#include "pokemon/patch/shiny.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/patch/app_status.h"

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

static bool PutInBox(u32 slot, SpeciesId species, FormId form,
                     FormId data_form) {
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
  scratch.is_illegal_egg = false;
  scratch.is_egg = false;
  scratch.SetShiny(false);
  scratch.experience =
      pokemon::Utils::GetExperienceFromLevel(species, data_form, 100);
  auto& pool = pokemon::Movepool::GetInstance(species, data_form);
  MoveId learnt[4] = {MoveId::kNone, MoveId::kNone, MoveId::kNone,
                      MoveId::kNone};
  u32 count = 0;
  for (u32 k = 0; k < pool.count; k++) {
    if (pool.entry[k].level > 100) break;
    learnt[count % 4] = pool.entry[k].move;
    count++;
  }
  scratch.SetMoves(learnt[0], learnt[1], learnt[2], learnt[3]);
  for (u32 k = 0; k < 4; k++) scratch.pp[k] = 20;
  scratch.ResetNickname();
  accessor.Encrypt();
  std::memcpy(&boxes.boxes[box].pokemons[index], &scratch, sizeof(scratch));
  return true;
}

static void FillBoxesWithGen7(void*) {
  if (savedata::PokemonTeam::GetInstance().count == 0) return;
  u32 slot = 0;
  for (u32 i = 0; i < pokemon::SpeciesTable::kGen7Count; i++) {
    const auto species =
        static_cast<SpeciesId>(pokemon::SpeciesTable::kFirstGen7Species + i);
    if (!PutInBox(slot, species, FormId::kNormal, FormId::kNormal)) break;
    slot++;
  }
  for (u16 i = 1; i <= pokemon::SpeciesTable::kSpeciesCount; i++) {
    if (!pokemon::AlolanForms::HasForm(i)) continue;
    if (!PutInBox(slot, static_cast<SpeciesId>(i), FormId::kMega,
                  FormId::kNormal))
      break;
    slot++;
  }
}

void LoadPokemonPage(MainApplication& app, void* args) {
  LoadShinyPage(app, args);

  app.Add("Randomize Gift Pokemon",
          overworld::GiftPokemon::GetInstance().randomize_species)
     .Add("Randomize Static Encounters",
          overworld::StaticRandomizer::GetInstance().randomize_species)
     .Add("Randomize Trades",
          overworld::Trade::GetInstance().randomize_species)
     .Add("Restricted Summary Editor", AppStatus::GetInstance().is_restricted)
     .AddSeparator()
     .Add("Fill Boxes With Gen 7 (Lv. 100)", FillBoxesWithGen7)
     .AddSeparator()
     .Add("Species Data", LoadSpeciesDataPage)
     .Add("Move Data", LoadMoveDataPage)
     .Add("Model Loader (Unstable)", LoadModelLoaderPage);
}
} // namespace ui