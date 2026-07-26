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
#include "feature/hook_manager.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"

namespace kaizo {
static const u16 SPECIAL_POKEMON[] = {
    // --- GENERATION 1 ---
    SPECIES_ARTICUNO, // Sub-Legendary (Allowed)
    SPECIES_ZAPDOS, // Sub-Legendary (Allowed)
    SPECIES_MOLTRES, // Sub-Legendary (Allowed)
    SPECIES_MEWTWO, // Restricted Legendary (Banned)
    SPECIES_MEW, // Mythical (Banned)

    // --- GENERATION 2 ---
    SPECIES_RAIKOU, // Sub-Legendary (Allowed)
    SPECIES_ENTEI, // Sub-Legendary (Allowed)
    SPECIES_SUICUNE, // Sub-Legendary (Allowed)
    SPECIES_LUGIA, // Restricted Legendary (Banned)
    SPECIES_HO_OH, // Restricted Legendary (Banned)
    SPECIES_CELEBI, // Mythical (Banned)

    // --- GENERATION 3 ---
    SPECIES_REGIROCK, // Sub-Legendary (Allowed)
    SPECIES_REGICE, // Sub-Legendary (Allowed)
    SPECIES_REGISTEEL, // Sub-Legendary (Allowed)
    SPECIES_LATIAS, // Sub-Legendary (Allowed)
    SPECIES_LATIOS, // Sub-Legendary (Allowed)
    SPECIES_KYOGRE, // Restricted Legendary (Banned)
    SPECIES_GROUDON, // Restricted Legendary (Banned)
    SPECIES_RAYQUAZA, // Restricted Legendary (Banned)
    SPECIES_JIRACHI, // Mythical (Banned)
    SPECIES_DEOXYS, // Mythical (Banned)

    // --- GENERATION 4 ---
    SPECIES_UXIE, // Sub-Legendary (Allowed)
    SPECIES_MESPRIT, // Sub-Legendary (Allowed)
    SPECIES_AZELF, // Sub-Legendary (Allowed)
    SPECIES_DIALGA, // Restricted Legendary (Banned)
    SPECIES_PALKIA, // Restricted Legendary (Banned)
    SPECIES_HEATRAN, // Sub-Legendary (Allowed)
    SPECIES_REGIGIGAS, // Sub-Legendary (Allowed)
    SPECIES_GIRATINA, // Restricted Legendary (Banned)
    SPECIES_CRESSELIA, // Sub-Legendary (Allowed)
    SPECIES_PHIONE, // Mythical (Banned)
    SPECIES_MANAPHY, // Mythical (Banned)
    SPECIES_DARKRAI, // Mythical (Banned)
    SPECIES_SHAYMIN, // Mythical (Banned)
    SPECIES_ARCEUS, // Mythical (Banned)

    // --- GENERATION 5 ---
    SPECIES_COBALION, // Sub-Legendary (Allowed)
    SPECIES_TERRAKION, // Sub-Legendary (Allowed)
    SPECIES_VIRIZION, // Sub-Legendary (Allowed)
    SPECIES_TORNADUS, // Sub-Legendary (Allowed)
    SPECIES_THUNDURUS, // Sub-Legendary (Allowed)
    SPECIES_RESHIRAM, // Restricted Legendary (Banned)
    SPECIES_ZEKROM, // Restricted Legendary (Banned)
    SPECIES_LANDORUS, // Sub-Legendary (Allowed)
    SPECIES_KYUREM, // Restricted Legendary (Banned)
    SPECIES_VICTINI, // Mythical (Banned)
    SPECIES_KELDEO, // Mythical (Banned)
    SPECIES_MELOETTA, // Mythical (Banned)
    SPECIES_GENESECT, // Mythical (Banned)

    // --- GENERATION 6 ---
    SPECIES_XERNEAS, // Restricted Legendary (Banned)
    SPECIES_YVELTAL, // Restricted Legendary (Banned)
    SPECIES_ZYGARDE, // Restricted Legendary (Banned)
    SPECIES_DIANCIE, // Mythical (Banned)
    SPECIES_HOOPA, // Mythical (Banned)
    SPECIES_VOLCANION, // Mythical (Banned)
};

bool IsSpecialPokemon(u16 species) {
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
    u16 species = pokemon->core->species;
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