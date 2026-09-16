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
  static_assert(SIZE(SHINY_RATES) == static_cast<u32>(pokemon::ShinyRate::kCount),
                "one label per ShinyRate");

  auto& shiny = pokemon::Shiny::GetInstance();

  app.Add("Shiny Rate", shiny.rate)
     .WithArray(SHINY_RATES, SIZE(SHINY_RATES))
     .WithBounds(0, SIZE(SHINY_RATES) - 1);
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
     .Add("Species Data", LoadSpeciesDataPage)
     .Add("Move Data", LoadMoveDataPage)
     .Add("Model Loader (Unstable)", LoadModelLoaderPage);
}
} // namespace ui
