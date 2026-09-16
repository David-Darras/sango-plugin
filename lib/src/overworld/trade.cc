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

#include "overworld/patch/trade.h"
#include "core/hook_manager.h"
#include "core/native/game_manager.h"
#include "pokemon/native/global_data/trade_pokemon.h"
#include "ui/log_application.h"
#include "core/utils.h"

namespace overworld {

void Trade::Initialize() {
  core::HookManager::Initialize(HookId::kTradePokemon, address::kTradePokemon,
                          (uptr)TradePokemonHook, false);
}

void Trade::PatchLoad() {
  core::HookManager::ForceEnable(HookId::kTradePokemon);
}

void Trade::RandomizeSpecies(u32 index) {
  auto& entry = global_data::TradePokemon::GetInstance(index);
  entry.species = core::Utils::GetRandomEnum<SpeciesId>();
  entry.form = Form::kNormal;
  entry.level = 1 + core::Utils::GetRandomValue(99);
  ui::LogApplication::Print(u"trade[%u]=%u", index, entry.species);
}

s32 Trade::TradePokemonHook(u32* p1, u32* p2) {
  if (GetInstance().randomize_species) RandomizeSpecies(p2[1]);
  return core::HookManager::Call<s32>(HookId::kTradePokemon, p1, p2);
}

} // namespace overworld
