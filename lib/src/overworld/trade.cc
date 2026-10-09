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
 * @file trade.cc
 * @brief Changes the in-game trades.
 *
 * The declarations are in overworld/patch/trade.h.
 */

#include "overworld/patch/trade.h"
#include "core/hook.h"
#include "pokemon/native/trade_pokemon_data.h"
#include "core/utils.h"

namespace overworld {

namespace {
core::Hook<s32(u32*, u32*)> trade_pokemon_hook;
} // namespace

void Trade::Initialize() {
  trade_pokemon_hook.Install(address::kTradePokemon, TradePokemonHook,
                             address::kVtable);
}

void Trade::RandomizeSpecies(u32 index) {
  auto& entry = pokemon::TradePokemonData::GetInstance(index);
  entry.species = core::Utils::GetRandomEnum<SpeciesId>();
  entry.form = FormId::kNormal;
  entry.level = 1 + core::Utils::GetRandomValue(99);
}

s32 Trade::TradePokemonHook(u32* p1, u32* p2) {
  if (GetInstance().randomize_species) RandomizeSpecies(p2[1]);
  return trade_pokemon_hook(p1, p2);
}

} // namespace overworld
