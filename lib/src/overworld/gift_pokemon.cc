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
#include "core/hook_manager.h"
#include "pokemon/native/global_data/gift_pokemon.h"
#include "ui/log_application.h"
#include "core/utils.h"

namespace overworld {

void GiftPokemon::Initialize() {
  core::HookManager::Initialize(HookId::kScriptAddPokemonToTeam,
                          pokemon::address::kScriptAddPokemonToTeam,
                          (uptr)ScriptAddPokemonToTeamHook, false);
}

void GiftPokemon::PatchLoad() {
  core::HookManager::ForceEnable(HookId::kScriptAddPokemonToTeam);
}

void GiftPokemon::RandomizeSpecies(u32 idx) {
  auto& entry = global_data::GiftPokemon::GetInstance(idx);
  entry.species = core::Utils::GetRandomEnum<SpeciesId>();
  entry.form = Form::kNormal;
  ui::LogApplication::Print(u"gift[%u]=%u", idx, entry.species);
}

s32 GiftPokemon::ScriptAddPokemonToTeamHook(u32* a1, u32* a2) {
  if (GetInstance().randomize_species) RandomizeSpecies(a2[1]);
  return core::HookManager::Call<s32>(HookId::kScriptAddPokemonToTeam, a1, a2);
}

} // namespace overworld
