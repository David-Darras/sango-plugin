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
#include "feature/core/hook_manager.h"

namespace feature {
class PokemonModel {
  MAKE_SINGLETON(PokemonModel)

public:
  void (*on_create)(PokeInfo* info) = nullptr;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kReplacePokemonModel,
                            ADDRESS_REPLACE_POKEMON_MODEL, (uptr)CreateHook);
  }

private:
  static void CreateHook(void* model, PokeInfo* info, void* p0, void* p1,
                         void* p2, void* p3) {
    auto& feat = GetInstance();
    if (feat.on_create != nullptr) feat.on_create(info);
    HookManager::Call<void>(HookID::kReplacePokemonModel, model, info, p0, p1,
                            p2, p3);
  }
};
} // namespace feature
