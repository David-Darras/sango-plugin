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
 * along with this program.  If not, see <https:
 */

#include "common.h"
#include "utils.h"
#include "feature/hook_manager.h"
#include "game/constant/species.h"
#include "game/savedata/pokemon_team.h"

namespace kaizo {
struct PokemonStarterView {
  PokeInfo info;
  u8 padding[0x54 - sizeof(PokeInfo)];
};

void PatchStarterView() {
  PokemonStarterView* data = (PokemonStarterView*)(ADDRESS_STARTER);
  data[0].info.is_egg = true;
  data[0].info.species = 0;
  data[1].info.is_egg = true;
  data[1].info.species = 0;
  data[2].info.is_egg = true;
  data[2].info.species = 0;
}

void PatchStarter(uptr pkm) {
  struct Pokemon {
    u64 id[3];
    u16 species;
    u8 form;
    u8 level;
  }& pokemon = *(Pokemon*)pkm;

  static const u16 GRASS_STARTERS[] = {
      SPECIES_BULBASAUR, SPECIES_CHIKORITA,
      SPECIES_TREECKO, SPECIES_TURTWIG,
      SPECIES_SNIVY, SPECIES_CHESPIN
  };

  u16 rand = Utils::GetRandomValue(SIZE(GRASS_STARTERS));
  switch (pokemon.species) {
    case SPECIES_TREECKO:
      pokemon.species = GRASS_STARTERS[rand] + 0;
      break;
    case SPECIES_TORCHIC:
      pokemon.species = GRASS_STARTERS[rand] + 3;
      break;
    case SPECIES_MUDKIP:
      pokemon.species = GRASS_STARTERS[rand] + 6;
      break;
  }
}

static u32 InitializePokemonHook(savedata::PokemonParam* param, u32 heap,
                                 uptr pkm) {
  PatchStarter(pkm);
  return HookManager::Call<
    u32>(HookID::kInitializePokemon, param, heap, pkm);
}

void InitializeStarterHook() {
  HookManager::Initialize(HookID::kInitializePokemon,
                          ADDRESS_INITIALIZE_POKEMON,
                          (uptr)InitializePokemonHook);
}
} // namespace kaizo