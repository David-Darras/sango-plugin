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

#include "pokemon/patch/evolution.h"
#include "core/hook_manager.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/constant/item.h"
#include "pokemon/native/global_data/evolution.h"

namespace pokemon {

void Evolution::Initialize() {
  core::HookManager::Initialize(HookId::kLoadEvolveTable,
                          address::kGlobalDataLoadEvolveTable,
                          (uptr)LoadEvolveTableHook);
}

void Evolution::LoadEvolveTableHook(SpeciesId species, u32 b, u32 c, u32 d) {
  core::HookManager::Call<void>(HookId::kLoadEvolveTable, species, b, c, d);
  PatchEvolve(species);
}

void Evolution::PatchEvolve(SpeciesId species) {
  auto& evolve_table = *(global_data::EvolutionTable*)
      READ32(address::kGlobalDataEvolveTable);
  auto& table = *evolve_table.data;
  switch (species) {
    case SpeciesId::kMachoke:
    case SpeciesId::kGraveler:
    case SpeciesId::kKadabra:
    case SpeciesId::kBoldore:
    case SpeciesId::kGurdurr:
    case SpeciesId::kHaunter:
    case SpeciesId::kKarrablast:
    case SpeciesId::kShelmet:
      table.data[0].level = 30;
      table.data[0].method = EvolutionMethod::kLevelUp;
      break;
    case SpeciesId::kSlowpoke:
    case SpeciesId::kPoliwhirl:
      table.data[0].item = ItemId::kKingsRock;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kScyther:
    case SpeciesId::kOnix:
      table.data[0].item = ItemId::kMetalCoat;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kSeadra:
      table.data[0].item = ItemId::kDragonScale;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kPorygon:
      table.data[0].item = ItemId::kUpGrade;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kPorygon2:
      table.data[0].item = ItemId::kDubiousDisc;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kClamperl:
      table.data[0].item = ItemId::kDeepSeaTooth;
      table.data[0].method = EvolutionMethod::kItem;
      table.data[1].item = ItemId::kDeepSeaScale;
      table.data[1].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kRhydon:
      table.data[0].item = ItemId::kProtector;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kElectabuzz:
      table.data[0].item = ItemId::kElectirizer;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kMagmar:
      table.data[0].item = ItemId::kMagmarizer;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kDusclops:
      table.data[0].item = ItemId::kReaperCloth;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kFeebas:
      table.data[0].item = ItemId::kPrismScale;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kSpritzee:
      table.data[0].item = ItemId::kSachet;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kSwirlix:
      table.data[0].item = ItemId::kWhippedDream;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    case SpeciesId::kPhantump:
    case SpeciesId::kPumpkaboo:
      table.data[0].item = ItemId::kLeafStone;
      table.data[0].method = EvolutionMethod::kItem;
      break;
    default:
      break;
  }
}

} // namespace pokemon
