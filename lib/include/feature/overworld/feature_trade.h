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
#include "game/core/manager.h"
#include "game/global_data/evolution.h"
#include "game/global_data/trade_pokemon.h"
#include "game/overworld/static_encounter.h"

namespace feature {
class Trade {
  MAKE_SINGLETON(Trade)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kTradePokemon, 0x0074AC64,
                            (uptr)TradePokemonHook, false);
  }

  STATIC_INLINE void PatchLoad() {
    HookManager::ForceEnable(HookID::kTradePokemon);
  }

private:
  static void RandomizeSpecies(u32 index) {
    auto& entry = global_data::TradePokemon::GetInstance(index);
    entry.species = static_cast<Species>
        (1 + Utils::GetRandomValue(static_cast<u32>(Species::kCount) - 1));
    entry.form = Form::kNormal;
    entry.level = 1 + Utils::GetRandomValue(99);
    ui::LogApplication::Print(u"trade[%u]=%u", index, entry.species);
  }

  static s32 TradePokemonHook(u32* p1, u32* p2) {
    RandomizeSpecies(p2[1]);
    return HookManager::Call<s32>(HookID::kTradePokemon, p1, p2);
  }
};
} // namespace feature