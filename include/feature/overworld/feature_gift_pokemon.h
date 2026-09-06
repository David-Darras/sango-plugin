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

#pragma once
#include "common.h"
#include "utils.h"
#include "feature/core/hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/species.h"
#include "game/global_data/gift_pokemon.h"

namespace feature {
class GiftPokemon {
  MAKE_SINGLETON(GiftPokemon)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kScriptAddPokemonToTeam, 0x0073F474,
                            (uptr)ScriptAddPokemonToTeamHook, false);
  }

  STATIC_INLINE void PatchLoad() {
    HookManager::ForceEnable(HookID::kScriptAddPokemonToTeam);
  }

private:
  static void RandomizeSpecies(u32 idx) {
    auto& entry = global_data::GiftPokemon::GetInstance(idx);
    entry.species = 1 + Utils::GetRandomValue(
                        static_cast<u32>(Species::kCount) - 1);
    entry.form = Form::kNormal;
  }

  static s32 ScriptAddPokemonToTeamHook(u32* a1, u32* a2) {
    RandomizeSpecies(a2[1]);
    return HookManager::Call<s32>(HookID::kScriptAddPokemonToTeam, a1, a2);
  }
};
} // namespace feature