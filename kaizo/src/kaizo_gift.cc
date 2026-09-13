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

#include "pokemon/constant/species.h"
#include "common.h"
#include "core/hook_manager.h"
#include "pokemon/native/data_accessor.h"
#include "savedata/native/pokemon_team.h"

namespace kaizo {
static const SpeciesId SPECIAL_POKEMON[] = {
    // --- GENERATION 1 ---
    SpeciesId::kArticuno, // Sub-Legendary (Allowed)
    SpeciesId::kZapdos, // Sub-Legendary (Allowed)
    SpeciesId::kMoltres, // Sub-Legendary (Allowed)
    SpeciesId::kMewtwo, // Restricted Legendary (Banned)
    SpeciesId::kMew, // Mythical (Banned)

    // --- GENERATION 2 ---
    SpeciesId::kRaikou, // Sub-Legendary (Allowed)
    SpeciesId::kEntei, // Sub-Legendary (Allowed)
    SpeciesId::kSuicune, // Sub-Legendary (Allowed)
    SpeciesId::kLugia, // Restricted Legendary (Banned)
    SpeciesId::kHoOh, // Restricted Legendary (Banned)
    SpeciesId::kCelebi, // Mythical (Banned)

    // --- GENERATION 3 ---
    SpeciesId::kRegirock, // Sub-Legendary (Allowed)
    SpeciesId::kRegice, // Sub-Legendary (Allowed)
    SpeciesId::kRegisteel, // Sub-Legendary (Allowed)
    SpeciesId::kLatias, // Sub-Legendary (Allowed)
    SpeciesId::kLatios, // Sub-Legendary (Allowed)
    SpeciesId::kKyogre, // Restricted Legendary (Banned)
    SpeciesId::kGroudon, // Restricted Legendary (Banned)
    SpeciesId::kRayquaza, // Restricted Legendary (Banned)
    SpeciesId::kJirachi, // Mythical (Banned)
    SpeciesId::kDeoxys, // Mythical (Banned)

    // --- GENERATION 4 ---
    SpeciesId::kUxie, // Sub-Legendary (Allowed)
    SpeciesId::kMesprit, // Sub-Legendary (Allowed)
    SpeciesId::kAzelf, // Sub-Legendary (Allowed)
    SpeciesId::kDialga, // Restricted Legendary (Banned)
    SpeciesId::kPalkia, // Restricted Legendary (Banned)
    SpeciesId::kHeatran, // Sub-Legendary (Allowed)
    SpeciesId::kRegigigas, // Sub-Legendary (Allowed)
    SpeciesId::kGiratina, // Restricted Legendary (Banned)
    SpeciesId::kCresselia, // Sub-Legendary (Allowed)
    SpeciesId::kPhione, // Mythical (Banned)
    SpeciesId::kManaphy, // Mythical (Banned)
    SpeciesId::kDarkrai, // Mythical (Banned)
    SpeciesId::kShaymin, // Mythical (Banned)
    SpeciesId::kArceus, // Mythical (Banned)

    // --- GENERATION 5 ---
    SpeciesId::kCobalion, // Sub-Legendary (Allowed)
    SpeciesId::kTerrakion, // Sub-Legendary (Allowed)
    SpeciesId::kVirizion, // Sub-Legendary (Allowed)
    SpeciesId::kTornadus, // Sub-Legendary (Allowed)
    SpeciesId::kThundurus, // Sub-Legendary (Allowed)
    SpeciesId::kReshiram, // Restricted Legendary (Banned)
    SpeciesId::kZekrom, // Restricted Legendary (Banned)
    SpeciesId::kLandorus, // Sub-Legendary (Allowed)
    SpeciesId::kKyurem, // Restricted Legendary (Banned)
    SpeciesId::kVictini, // Mythical (Banned)
    SpeciesId::kKeldeo, // Mythical (Banned)
    SpeciesId::kMeloetta, // Mythical (Banned)
    SpeciesId::kGenesect, // Mythical (Banned)

    // --- GENERATION 6 ---
    SpeciesId::kXerneas, // Restricted Legendary (Banned)
    SpeciesId::kYveltal, // Restricted Legendary (Banned)
    SpeciesId::kZygarde, // Restricted Legendary (Banned)
    SpeciesId::kDiancie, // Mythical (Banned)
    SpeciesId::kHoopa, // Mythical (Banned)
    SpeciesId::kVolcanion, // Mythical (Banned)
};

bool IsSpecialPokemon(SpeciesId species) {
  for (u32 i = 0; i < SIZE(SPECIAL_POKEMON); i++) {
    if (species == SPECIAL_POKEMON[i]) {
      return true;
    }
  }
  return false;
}

static bool AddPokemonToTeamHook(savedata::PokemonTeam* team,
                                 savedata::PokemonParam* pokemon) {
  if (team == &savedata::PokemonTeam::GetInstance()) {
    pokemon->accessor->Decrypt();
    SpeciesId species = pokemon->core->species;
    pokemon->accessor->Encrypt();

    if (IsSpecialPokemon(species)) {
      return false;
    }
  }

  return core::HookManager::Call<bool>(HookId::kAddPokemonToTeam, team, pokemon);
}

void InitializeGiftHook() {
  core::HookManager::Initialize(HookId::kAddPokemonToTeam,
                          pokemon::address::kAddPokemonToTeam,
                          (uptr)AddPokemonToTeamHook);
}
}