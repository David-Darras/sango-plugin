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
#include "pokemon/patch/species_packs.h"
#include "pokemon/address.h"

namespace pokemon {

namespace {
core::Hook<u32(void*, void*)> get_mega_evolved_form_no_hook;
} // namespace

static constexpr u32 kMaxRoutes = 3;

void AlolanForms::Initialize() {
  if (address::kGetMegaEvolvedFormNo == 0) return;
  get_mega_evolved_form_no_hook.Install(address::kGetMegaEvolvedFormNo,
                                        GetMegaEvolvedFormNoHook);
}

// The extra forms are in the file that Sango Studio writes (see
// pokemon/patch/species_packs.h). They keep the order of the file.
bool AlolanForms::HasForm(u16 species) {
  return FormCount(species) != 0;
}

u32 AlolanForms::FormCount(u16 species) {
  SpeciesPacks::Load();
  u32 count = 0;
  for (u32 i = 0; i < SpeciesPacks::FormCount(); i++) {
    if (SpeciesPacks::GetForm(i).species == species) count++;
  }
  return count;
}

u32 AlolanForms::GetForm(u16 species, u32 rank) {
  SpeciesPacks::Load();
  for (u32 i = 0; i < SpeciesPacks::FormCount(); i++) {
    if (SpeciesPacks::GetForm(i).species != species) continue;
    if (rank == 0) return SpeciesPacks::GetForm(i).form;
    rank--;
  }
  return 0;
}

u32 AlolanForms::GetModelForm(u16 species, u32 form) {
  SpeciesPacks::Load();
  for (u32 i = 0; i < SpeciesPacks::FormCount(); i++) {
    const PackForm& entry = SpeciesPacks::GetForm(i);
    if (entry.species == species && entry.form == form) {
      return entry.model_form;
    }
  }
  return 0;
}

namespace {
// The kinds of condition in the file (the 3 high bits of PackForm::mega).
constexpr u16 kNever = 0;
constexpr u16 kNoCondition = 1;
constexpr u16 kHeldItem = 2;
constexpr u16 kKnownMove = 3;
constexpr u16 kAbility = 4;
constexpr u16 kNumberMask = 0x1FFF;

/// Returns true when the Pokémon meets the condition of a form.
bool MeetsCondition(u16 condition, void* pokemon) {
  const u16 kind = condition >> 13;
  const u16 number = condition & kNumberMask;
  switch (kind) {
  case kNoCondition:
    return true;
  case kHeldItem:
    return core::GameFunction<u16(void*)>::Call(address::kCoreDataGetItem,
                                                pokemon) == number;
  case kKnownMove:
    return core::GameFunction<bool(void*, u16)>::Call(
        address::kCoreDataHasMove, pokemon, number);
  case kAbility: {
    void* data = reinterpret_cast<void*>(READ32(
        reinterpret_cast<uptr>(pokemon) + 12));
    return core::GameFunction<u8(void*)>::Call(
               address::kPokemonDataGetAbility, data) == number;
  }
  default: // kNever and the kinds that this plugin does not know.
    return false;
  }
}
} // namespace

u32 AlolanForms::FindMegaForm(u16 species, void* pokemon) {
  SpeciesPacks::Load();
  for (u32 i = 0; i < SpeciesPacks::FormCount(); i++) {
    const PackForm& entry = SpeciesPacks::GetForm(i);
    if (entry.species == species && entry.mega != kNever &&
        MeetsCondition(entry.mega, pokemon)) {
      return entry.form;
    }
  }
  return 0;
}

bool AlolanForms::IsLifeOrbSpecies(u16 species) {
  // No species keeps the Life Orb for a model of the game alone.
  (void)species;
  return false;
}

u32 AlolanForms::GetMegaEvolvedFormNoHook(void* manager, void* poke) {
  if (poke != nullptr) {
    const u16 species =
        core::GameFunction<u16(void*)>::Call(
            address::kCoreDataGetSpecies, poke);
    if (HasForm(species)) {
      const u32 form = FindMegaForm(species, poke);
      if (form != 0) return form;
    }
  }
  return get_mega_evolved_form_no_hook(manager, poke);
}

bool AlolanForms::PatchMegaTable(SpeciesId species, MegaEvolutionData* table) {
  const u32 count = FormCount(static_cast<u16>(species));
  if (count != 0) {
    if (SpeciesTable::IsAddedSpecies(static_cast<u16>(species))) std::memset(table, 0, sizeof(*table));
    // Each form of the species that has an item is a route, in the order of
    // the file (Sango Studio gives the item of each form).
    u32 route = 0;
    for (u32 i = 0; i < SpeciesPacks::FormCount(); i++) {
      const PackForm& entry = SpeciesPacks::GetForm(i);
      if (entry.species != static_cast<u16>(species) || entry.mega == kNever) {
        continue;
      }
      while (route < kMaxRoutes &&
             table->entry[route].method != MegaEvolutionMethod::kNone) {
        route++;
      }
      if (route >= kMaxRoutes) break;
      table->entry[route].form = static_cast<FormId>(entry.form);
      if ((entry.mega >> 13) == kHeldItem) {
        // The game shows that the item is a Mega Stone of the species.
        table->entry[route].method = MegaEvolutionMethod::kItem;
        table->entry[route].item = static_cast<ItemId>(entry.mega & kNumberMask);
      } else {
        // The other conditions are checked by FindMegaForm().
        table->entry[route].method = MegaEvolutionMethod::kRayquaza;
        table->entry[route].item = ItemId::kNone;
      }
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
