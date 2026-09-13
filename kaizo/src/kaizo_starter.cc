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
#include "core/utils.h"
#include "core/hook_manager.h"
#include "pokemon/constant/species.h"
#include "savedata/native/pokemon_team.h"

namespace kaizo {
struct PokemonStarterView {
  PokeInfo info;
  u8 padding[0x54 - sizeof(PokeInfo)];
};

void PatchStarterView() {
  PokemonStarterView* data = (PokemonStarterView*)(core::address::kStarter);
  data[0].info.is_egg = true;
  data[0].info.species = SpeciesId::kNone;
  data[1].info.is_egg = true;
  data[1].info.species = SpeciesId::kNone;
  data[2].info.is_egg = true;
  data[2].info.species = SpeciesId::kNone;
}

void PatchStarter(uptr pkm) {
  struct Pokemon {
    u64 id[3];
    u16 species;
    u8 form;
    u8 level;
  }& pokemon = *(Pokemon*)pkm;

  static const SpeciesId GRASS_STARTERS[] = {
      SpeciesId::kBulbasaur, SpeciesId::kChikorita,
      SpeciesId::kTreecko, SpeciesId::kTurtwig,
      SpeciesId::kSnivy, SpeciesId::kChespin
  };

  u16 rand = core::Utils::GetRandomValue(SIZE(GRASS_STARTERS));
  switch (static_cast<SpeciesId>(pokemon.species)) {
    case SpeciesId::kTreecko:
      pokemon.species = static_cast<u16>(GRASS_STARTERS[rand]) + 0;
      break;
    case SpeciesId::kTorchic:
      pokemon.species = static_cast<u16>(GRASS_STARTERS[rand]) + 3;
      break;
    case SpeciesId::kMudkip:
      pokemon.species = static_cast<u16>(GRASS_STARTERS[rand]) + 6;
      break;
  }
}

static u32 InitializePokemonHook(savedata::PokemonParam* param, u32 heap,
                                 uptr pkm) {
  PatchStarter(pkm);
  return core::HookManager::Call<u32>(HookId::kInitializePokemon, param, heap, pkm);
}

void InitializeStarterHook() {
  core::HookManager::Initialize(HookId::kInitializePokemon,
                          pokemon::address::kInitializePokemon,
                          (uptr)InitializePokemonHook);
}
} // namespace kaizo