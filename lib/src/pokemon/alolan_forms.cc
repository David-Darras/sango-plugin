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
 * @file alolan_forms.cc
 * @brief Adds extra forms to species: Alolan, Galarian, new Mega Evolutions...
 *
 * The declarations are in pokemon/patch/alolan_forms.h.
 */

#include "pokemon/patch/alolan_forms.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/mega_evolution_method.h"
#include "pokemon/native/mega_evolution_data.h"
#include <cstring>
#include "core/hook.h"
#include "pokemon/patch/species_table.h"
#include "pokemon/data/alolan_form.inc"

namespace pokemon {

namespace {
core::Hook<u32(void*, void*)> get_mega_evolved_form_no_hook;
} // namespace

static constexpr ItemId kFormItems[] = {ItemId::kLifeOrb, ItemId::kChoiceBand,
                                        ItemId::kChoiceSpecs, ItemId::kChoiceScarf};
static constexpr u32 kMaxRoutes = 3;

void AlolanForms::Initialize() {
  if (address::kGetMegaEvolvedFormNo == 0) return;
  get_mega_evolved_form_no_hook.Install(address::kGetMegaEvolvedFormNo,
                                        GetMegaEvolvedFormNoHook);
}

bool AlolanForms::HasForm(u16 species) {
  return FormCount(species) != 0;
}

u32 AlolanForms::FormCount(u16 species) {
  u32 count = 0;
  for (u32 i = 0; i < kFormCount; i++) {
    if (kAlolanForms[i].species == species) count++;
  }
  return count;
}

u32 AlolanForms::GetForm(u16 species, u32 rank) {
  for (u32 i = 0; i < kFormCount; i++) {
    if (kAlolanForms[i].species != species) continue;
    if (rank == 0) return kAlolanForms[i].form;
    rank--;
  }
  return 0;
}

u32 AlolanForms::GetModelForm(u16 species, u32 form) {
  for (u32 i = 0; i < kFormCount; i++) {
    if (kAlolanForms[i].species == species && kAlolanForms[i].form == form) {
      return kAlolanForms[i].model_form;
    }
  }
  return 0;
}

ItemId AlolanForms::GetItem(u32 rank) {
  return rank < SIZE(kFormItems) ? kFormItems[rank] : ItemId::kNone;
}

u32 AlolanForms::GetFormByItem(u16 species, ItemId item) {
  for (u32 rank = 0; rank < SIZE(kFormItems); rank++) {
    if (kFormItems[rank] == item) return GetForm(species, rank);
  }
  return 0;
}

bool AlolanForms::IsLifeOrbSpecies(u16 species) {
  for (u32 i = 0; i < kLifeOrbCount; i++) {
    if (kLifeOrbSpecies[i] == species) return true;
  }
  return false;
}

u32 AlolanForms::GetMegaEvolvedFormNoHook(void* manager, void* poke) {
  if (poke != nullptr) {
    const u16 species =
        ((u16 (*)(void*))address::kCoreDataGetSpecies)(poke);
    if (HasForm(species)) {
      const auto item = static_cast<ItemId>(
          ((u16 (*)(void*))address::kCoreDataGetItem)(poke));
      const u32 form = GetFormByItem(species, item);
      if (form != 0) return form;
    }
  }
  return get_mega_evolved_form_no_hook(manager, poke);
}

bool AlolanForms::PatchMegaTable(SpeciesId species, MegaEvolutionData* table) {
  const u32 count = FormCount(static_cast<u16>(species));
  if (count != 0) {
    if (SpeciesTable::IsGen7(static_cast<u16>(species))) std::memset(table, 0, sizeof(*table));
    u32 route = 0;
    for (u32 rank = 0; rank < count; rank++) {
      while (route < kMaxRoutes &&
             table->entry[route].method != MegaEvolutionMethod::kNone) {
        route++;
      }
      if (route >= kMaxRoutes) break;
      table->entry[route].form =
          static_cast<FormId>(GetForm(static_cast<u16>(species), rank));
      table->entry[route].method = MegaEvolutionMethod::kItem;
      table->entry[route].item = GetItem(rank);
      route++;
    }
    return true;
  }
  if (IsLifeOrbSpecies(static_cast<u16>(species))) {
    table->entry[0].method = MegaEvolutionMethod::kItem;
    table->entry[0].item = ItemId::kLifeOrb;
    return true;
  }
  return false;
}

} // namespace pokemon
