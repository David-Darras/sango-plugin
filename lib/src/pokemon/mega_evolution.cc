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

#include "pokemon/patch/mega_evolution.h"
#include "core/hook_manager.h"
#include "pokemon/constant/item.h"
#include "pokemon/native/global_data/mega_evolution.h"

namespace pokemon {

void MegaEvolution::Initialize() {
  core::HookManager::Initialize(HookId::kGlobalDataLoadMegaEvolutionTable,
                          address::kGlobalDataLoadMegaEvolutionTable,
                          (uptr)LoadMegaEvolutionTableHook);
  // disable cache
  ARM_RET(address::kGlobalDataLoadMegaEvolutionTable + 0x3C);
}

void MegaEvolution::LoadMegaEvolutionTableHook(SpeciesId species) {
  core::HookManager::Call<void>(HookId::kGlobalDataLoadMegaEvolutionTable, species);

  auto& mega_evolve_table = *(global_data::MegaEvolutionTable*)
      READ32(address::kGlobalDataMegaEvolutionTable);
  auto& table = *mega_evolve_table.data;
  if (species == SpeciesId::kMimeJr) {
    table.entry[0].form = kFormMimeJrMega;
    table.entry[0].method = MegaEvolutionMethod::kItem;
    table.entry[0].item = ItemId::kLifeOrb;
  }
}

} // namespace pokemon
