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

#include "pokemon/patch/model_replacement.h"
#include "core/hook_manager.h"

namespace pokemon {

void ModelReplacement::Initialize() {
  core::HookManager::Initialize(HookId::kReplacePokemonModel,
                          address::kReplacePokemonModel, (uptr)CreateHook);
}

void ModelReplacement::CreateHook(void* model, PokeInfo* info, void* p0, void* p1,
                              void* p2, void* p3) {
  auto& feat = GetInstance();
  if (feat.on_create != nullptr) feat.on_create(info);
  core::HookManager::Call<void>(HookId::kReplacePokemonModel, model, info, p0, p1,
                          p2, p3);
}

} // namespace pokemon
