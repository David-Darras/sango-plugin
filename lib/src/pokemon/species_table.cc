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

#include "pokemon/patch/species_table.h"
#include "pokemon/patch/alolan_forms.h"
#include <cstring>
#include "pokemon/constant/ability.h"
#include "pokemon/native/database.h"
#include "pokemon/native/poke_info.h"
#include "pokemon/native/evolution_data.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/species_data.h"
#include "core/hook_manager.h"
#include "core/memory.h"
#include "pokemon/data/alolan_form.inc"
#include "pokemon/data/gen7_species.inc"
#include "pokemon/data/gen8_species.inc"

namespace pokemon {

static constexpr u16 kGen9Carrier = 721;
static constexpr u32 kSecondCarrierBase = 256;
static constexpr u32 kNativeModel = 0x8000;
static constexpr u32 kGen7Base = SpeciesTable::kSpeciesCount + 3;
static constexpr u32 kAlolanFormCount = sizeof(kAlolanForms) / sizeof(kAlolanForms[0]);

static const Gen7Species& Extra(u32 i) {
  return i < SpeciesTable::kGen7Count ? kGen7Species[i]
                                      : kGen8Species[i - SpeciesTable::kGen7Count];
}

u16 SpeciesTable::kFirstGen7Species = kGen7Species[0].species;

bool SpeciesTable::IsGen7(u16 species) {
  for (u32 i = 0; i < SpeciesTable::kExtraCount; i++) {
    if (Extra(i).species == species) return true;
  }
  return false;
}

void SpeciesTable::PatchModelRequest(PokeInfo* info) {
  if (info == nullptr) return;
  const u16 species = static_cast<u16>(info->species);
  for (u32 i = 0; i < SpeciesTable::kExtraCount; i++) {
    if (Extra(i).species != species) continue;
    const u32 form_model = info->form != FormId::kNormal
        ? AlolanForms::GetModelForm(species, static_cast<u32>(info->form)) : 0;
    const u32 model = form_model != 0 ? form_model : Extra(i).model_form;
    if ((model & kNativeModel) != 0) {
      info->species = static_cast<SpeciesId>(model & ~kNativeModel);
      info->form = FormId::kNormal;
    } else if (model >= kSecondCarrierBase) {
      info->species = static_cast<SpeciesId>(kGen9Carrier);
      info->form = static_cast<FormId>(model - kSecondCarrierBase);
    } else {
      info->species = static_cast<SpeciesId>(kGen7Carrier);
      info->form = static_cast<FormId>(model);
    }
    return;
  }
}

static void FixAbilities(SpeciesData* entry, const SpeciesData* fallback) {
  for (u32 k = 0; k < 3; k++) {
    if (static_cast<u8>(entry->ability[k]) < static_cast<u8>(AbilityId::kCount)) {
      continue;
    }
    entry->ability[k] =
        static_cast<u8>(entry->ability[0]) < static_cast<u8>(AbilityId::kCount)
          ? entry->ability[0]
          : fallback->ability[0];
  }
}

void SpeciesTable::BuildTable(u8* game_table) {
  auto& feat = GetInstance();
  u8* out = feat.table_;

  const u32 species_entries = kGen7Base;
  const u32 form_entries = kEntries - species_entries;
  std::memcpy(out, game_table, species_entries * kEntrySize);
  std::memcpy(out + (species_entries + SpeciesTable::kExtraCount) * kEntrySize,
              game_table + species_entries * kEntrySize,
              form_entries * kEntrySize);

  for (u32 i = 0; i < species_entries; i++) {
    auto* entry = (SpeciesData*)(out + i * kEntrySize);
    if (entry->form_index != 0) entry->form_index += SpeciesTable::kExtraCount;
    if (entry->form_index_2 != 0) entry->form_index_2 += SpeciesTable::kExtraCount;
  }

  for (u32 i = 0; i < SpeciesTable::kExtraCount; i++) {
    auto* entry = (SpeciesData*)(out + (kGen7Base + i) * kEntrySize);
    std::memcpy(entry, Extra(i).data, kEntrySize);
    FixAbilities(entry, (SpeciesData*)out);
    entry->form_index = 0;
    entry->form_index_2 = 0;
    entry->form_count = 1;
  }

  u32 next = kEntries + SpeciesTable::kExtraCount;
  for (u32 i = 0; i < kAlolanFormCount; i++) {
    const AlolanForm& form = kAlolanForms[i];
    auto* base = (SpeciesData*)(out + form.species * kEntrySize);
    const u32 existing = base->form_count > 1 ? base->form_count - 1u : 0u;
    const u32 first = next;
    for (u32 k = 0; k < existing; k++) {
      std::memcpy(out + next * kEntrySize,
                  out + (base->form_index + k) * kEntrySize, kEntrySize);
      next++;
    }
    auto* entry = (SpeciesData*)(out + next * kEntrySize);
    std::memcpy(entry, form.data, kEntrySize);
    FixAbilities(entry, base);
    entry->form_index = 0;
    entry->form_index_2 = 0;
    entry->form_count = 1;
    base->form_index = (u16)first;
    base->form_index_2 = (u16)first;
    base->form_count = (u8)(existing + 2);
    next++;
  }
}

void SpeciesTable::Update() {
#ifdef GAME_ORAS
  auto& feat = GetInstance();
  if (feat.is_built_) return;
  auto& db = Database::GetInstance();
  u8* game_table = (u8*)db.species;
  if (game_table == nullptr) return;
  feat.is_built_ = true;
  BuildTable(game_table);
  db.species = (SpeciesData*)feat.table_;
#endif
}

void SpeciesTable::GetSpeciesNameHook(String* output, u16 species) {
  for (u32 i = 0; i < SpeciesTable::kExtraCount; i++) {
    if (Extra(i).species != species) continue;
    if (output != nullptr) output->Set(Extra(i).name);
    return;
  }
  core::HookManager::Call<void>(HookId::kGetSpeciesName, output, species);
}

void SpeciesTable::LoadMovepoolHook(u16 species, u8 form) {
  for (u32 i = 0; i < SpeciesTable::kExtraCount; i++) {
    if (Extra(i).species != species) continue;
    core::HookManager::Call<void>(HookId::kLoadMovepool, 1, 0);
    auto& pool = Movepool::Object();
    pool.species = static_cast<SpeciesId>(species);
    pool.form = static_cast<FormId>(form);
    u32 count = 0;
    for (u32 k = 0; k < Gen7Species::kMoveMax; k++) {
      const u16 move = Extra(i).moves[k].move;
      if (move == 0) break;
      if (move >= static_cast<u16>(MoveId::kCount)) continue;
      pool.entry[count].move = static_cast<MoveId>(move);
      pool.entry[count].level = Extra(i).moves[k].level;
      pool.entry[count].padding = 0;
      count++;
    }
    pool.count = count;
    return;
  }
  core::HookManager::Call<void>(HookId::kLoadMovepool, species, form);
}

bool SpeciesTable::PatchEvolutionTable(u16 species, EvolutionData* table) {
  for (u32 i = 0; i < SpeciesTable::kExtraCount; i++) {
    if (Extra(i).species != species) continue;
    std::memset(table, 0, sizeof(*table));
    for (u32 k = 0; k < Gen7Species::kEvolutionMax; k++) {
      const Gen7Evolution& evolution = Extra(i).evolutions[k];
      table->data[k].method = static_cast<EvolutionMethod>(evolution.method);
      table->data[k].arg = evolution.arg;
      table->data[k].species = static_cast<SpeciesId>(evolution.species);
    }
    return true;
  }
  return false;
}

void SpeciesTable::Initialize() {
  for (u32 i = 0; i < SIZE(address::kSpeciesBound); i++) {
    if (address::kSpeciesBound[i] == 0) continue;
    WRITE32(address::kSpeciesBound[i], kTotalEntries - 1);
  }
  core::HookManager::Initialize(HookId::kGetSpeciesName,
                                address::kGetSpeciesName,
                                (uptr)GetSpeciesNameHook);
  core::HookManager::Initialize(HookId::kLoadMovepool, address::kLoadMovepool,
                                (uptr)LoadMovepoolHook);
}

} // namespace pokemon
