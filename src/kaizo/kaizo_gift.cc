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

#include "game/constant/species.h"
#include "common.h"
#include "feature/core/hook_manager.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"

namespace kaizo {
static const Species SPECIAL_POKEMON[] = {
    // --- GENERATION 1 ---
    Species::kArticuno, // Sub-Legendary (Allowed)
    Species::kZapdos, // Sub-Legendary (Allowed)
    Species::kMoltres, // Sub-Legendary (Allowed)
    Species::kMewtwo, // Restricted Legendary (Banned)
    Species::kMew, // Mythical (Banned)

    // --- GENERATION 2 ---
    Species::kRaikou, // Sub-Legendary (Allowed)
    Species::kEntei, // Sub-Legendary (Allowed)
    Species::kSuicune, // Sub-Legendary (Allowed)
    Species::kLugia, // Restricted Legendary (Banned)
    Species::kHoOh, // Restricted Legendary (Banned)
    Species::kCelebi, // Mythical (Banned)

    // --- GENERATION 3 ---
    Species::kRegirock, // Sub-Legendary (Allowed)
    Species::kRegice, // Sub-Legendary (Allowed)
    Species::kRegisteel, // Sub-Legendary (Allowed)
    Species::kLatias, // Sub-Legendary (Allowed)
    Species::kLatios, // Sub-Legendary (Allowed)
    Species::kKyogre, // Restricted Legendary (Banned)
    Species::kGroudon, // Restricted Legendary (Banned)
    Species::kRayquaza, // Restricted Legendary (Banned)
    Species::kJirachi, // Mythical (Banned)
    Species::kDeoxys, // Mythical (Banned)

    // --- GENERATION 4 ---
    Species::kUxie, // Sub-Legendary (Allowed)
    Species::kMesprit, // Sub-Legendary (Allowed)
    Species::kAzelf, // Sub-Legendary (Allowed)
    Species::kDialga, // Restricted Legendary (Banned)
    Species::kPalkia, // Restricted Legendary (Banned)
    Species::kHeatran, // Sub-Legendary (Allowed)
    Species::kRegigigas, // Sub-Legendary (Allowed)
    Species::kGiratina, // Restricted Legendary (Banned)
    Species::kCresselia, // Sub-Legendary (Allowed)
    Species::kPhione, // Mythical (Banned)
    Species::kManaphy, // Mythical (Banned)
    Species::kDarkrai, // Mythical (Banned)
    Species::kShaymin, // Mythical (Banned)
    Species::kArceus, // Mythical (Banned)

    // --- GENERATION 5 ---
    Species::kCobalion, // Sub-Legendary (Allowed)
    Species::kTerrakion, // Sub-Legendary (Allowed)
    Species::kVirizion, // Sub-Legendary (Allowed)
    Species::kTornadus, // Sub-Legendary (Allowed)
    Species::kThundurus, // Sub-Legendary (Allowed)
    Species::kReshiram, // Restricted Legendary (Banned)
    Species::kZekrom, // Restricted Legendary (Banned)
    Species::kLandorus, // Sub-Legendary (Allowed)
    Species::kKyurem, // Restricted Legendary (Banned)
    Species::kVictini, // Mythical (Banned)
    Species::kKeldeo, // Mythical (Banned)
    Species::kMeloetta, // Mythical (Banned)
    Species::kGenesect, // Mythical (Banned)

    // --- GENERATION 6 ---
    Species::kXerneas, // Restricted Legendary (Banned)
    Species::kYveltal, // Restricted Legendary (Banned)
    Species::kZygarde, // Restricted Legendary (Banned)
    Species::kDiancie, // Mythical (Banned)
    Species::kHoopa, // Mythical (Banned)
    Species::kVolcanion, // Mythical (Banned)
};

bool IsSpecialPokemon(Species species) {
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
    Species species = pokemon->core->species;
    pokemon->accessor->Encrypt();

    if (IsSpecialPokemon(species)) {
      return false;
    }
  }

  return HookManager::Call<bool>(HookID::kAddPokemonToTeam, team, pokemon);
}

void InitializeGiftHook() {
  HookManager::Initialize(HookID::kAddPokemonToTeam,
                          ADDRESS_ADD_POKEMON_TO_TEAM,
                          (uptr)AddPokemonToTeamHook);
}
}