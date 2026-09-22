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

#include "pokemon/patch/alolan_forms.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/mega_evolution_method.h"
#include "pokemon/native/mega_evolution_data.h"
#include "core/hook_manager.h"
#include "pokemon/data/alolan_form.inc"

namespace pokemon {

static constexpr ItemId kMegaItem = ItemId::kLifeOrb;

void AlolanForms::Initialize() {
  if (address::kGetMegaEvolvedFormNo == 0) return;
  core::HookManager::Initialize(HookId::kGetMegaEvolvedFormNo,
                                address::kGetMegaEvolvedFormNo,
                                (uptr)GetMegaEvolvedFormNoHook);
}

bool AlolanForms::HasForm(u16 species) {
  for (u32 i = 0; i < kFormCount; i++) {
    if (kAlolanForms[i].species == species) return true;
  }
  return false;
}

u32 AlolanForms::GetMegaEvolvedFormNoHook(void* manager, void* poke) {
  if (poke != nullptr) {
    const u16 species =
        ((u16 (*)(void*))address::kCoreParamGetSpecies)(poke);
    if (HasForm(species)) return 1;
  }
  return core::HookManager::Call<u32>(HookId::kGetMegaEvolvedFormNo, manager,
                                      poke);
}

bool AlolanForms::PatchMegaTable(SpeciesId species, MegaEvolutionData* table) {
  for (u32 i = 0; i < kFormCount; i++) {
    if (kAlolanForms[i].species != static_cast<u16>(species)) continue;
    table->entry[0].form = static_cast<FormId>(1);
    table->entry[0].method = MegaEvolutionMethod::kItem;
    table->entry[0].item = kMegaItem;
    return true;
  }
  return false;
}

} // namespace pokemon
