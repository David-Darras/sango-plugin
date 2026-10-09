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
 * @file kaizo_gift.cc
 * @brief Kaizo: the legendary Pokémon cannot join the party (a hook on AddPokemonToTeam).
 */

#include "pokemon/constant/species.h"
#include "common.h"
#include "core/hook.h"
#include "savedata/native/pokemon_team.h"

namespace kaizo {
static const SpeciesId SPECIAL_POKEMON[] = {
    // Generation I.
    SpeciesId::kArticuno,
    SpeciesId::kZapdos,
    SpeciesId::kMoltres,
    SpeciesId::kMewtwo,
    SpeciesId::kMew,

    // Generation II.
    SpeciesId::kRaikou,
    SpeciesId::kEntei,
    SpeciesId::kSuicune,
    SpeciesId::kLugia,
    SpeciesId::kHoOh,
    SpeciesId::kCelebi,

    // Generation III.
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

    // Generation IV.
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

    // Generation V.
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

    // Generation VI.
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

// The legendary Pokémon cannot join the party of the player.
bool, AddPokemonToTeam,
     (savedata::PokemonTeam* team, savedata::PokemonParam* pokemon),
     pokemon::address::kAddPokemonToTeam) {
  if (team == &savedata::PokemonTeam::GetInstance()) {
    pokemon->accessor->Decrypt();
    SpeciesId species = pokemon->core->species;
    pokemon->accessor->Encrypt();

    if (IsSpecialPokemon(species)) {
      return false;
    }
  }

  return original(team, pokemon);
}
} // namespace kaizo
