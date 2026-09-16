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
#include "savedata/native/pokemon_team.h"

namespace kaizo {
static const SpeciesId SPECIAL_POKEMON[] = {
    // --- GENERATION 1 ---
    SpeciesId::kArticuno,
    SpeciesId::kZapdos,
    SpeciesId::kMoltres,
    SpeciesId::kMewtwo,
    SpeciesId::kMew,

    // --- GENERATION 2 ---
    SpeciesId::kRaikou,
    SpeciesId::kEntei,
    SpeciesId::kSuicune,
    SpeciesId::kLugia,
    SpeciesId::kHoOh,
    SpeciesId::kCelebi,

    // --- GENERATION 3 ---
    SpeciesId::kRegirock,
    SpeciesId::kRegice,
    SpeciesId::kRegisteel,
    SpeciesId::kLatias,
    SpeciesId::kLatios,
    SpeciesId::kKyogre,
    SpeciesId::kGroudon,
    SpeciesId::kRayquaza,
    SpeciesId::kJirachi,
    SpeciesId::kDeoxys,

    // --- GENERATION 4 ---
    SpeciesId::kUxie,
    SpeciesId::kMesprit,
    SpeciesId::kAzelf,
    SpeciesId::kDialga,
    SpeciesId::kPalkia,
    SpeciesId::kHeatran,
    SpeciesId::kRegigigas,
    SpeciesId::kGiratina,
    SpeciesId::kCresselia,
    SpeciesId::kPhione,
    SpeciesId::kManaphy,
    SpeciesId::kDarkrai,
    SpeciesId::kShaymin,
    SpeciesId::kArceus,

    // --- GENERATION 5 ---
    SpeciesId::kCobalion,
    SpeciesId::kTerrakion,
    SpeciesId::kVirizion,
    SpeciesId::kTornadus,
    SpeciesId::kThundurus,
    SpeciesId::kReshiram,
    SpeciesId::kZekrom,
    SpeciesId::kLandorus,
    SpeciesId::kKyurem,
    SpeciesId::kVictini,
    SpeciesId::kKeldeo,
    SpeciesId::kMeloetta,
    SpeciesId::kGenesect,

    // --- GENERATION 6 ---
    SpeciesId::kXerneas,
    SpeciesId::kYveltal,
    SpeciesId::kZygarde,
    SpeciesId::kDiancie,
    SpeciesId::kHoopa,
    SpeciesId::kVolcanion,
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