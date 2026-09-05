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

#include "common.h"
#include "feature/core/hook_manager.h"
#include "game/constant/evolution_method.h"
#include "game/constant/item.h"
#include "game/constant/species.h"
#include "game/global_data/evolve.h"

namespace global_data {
struct Evolve;
}

namespace kaizo {
struct Table {
  void* vtable;
  u16 species;
  global_data::Evolve* evolve;
};

void PatchEvolve(Species species) {
  Table* table = (Table*)READ32(ADDRESS_GLOBAL_DATA_EVOLVE_TABLE);
  auto& evolve = *table->evolve;
  switch (species) {
    case Species::kMachoke:
    case Species::kGraveler:
    case Species::kKadabra:
    case Species::kBoldore:
    case Species::kGurdurr:
    case Species::kHaunter:
    case Species::kKarrablast:
    case Species::kShelmet:
      evolve.data[0].arg = 30; // level
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kLevelUp);
      break;
    case Species::kSlowpoke:
    case Species::kPoliwhirl:
      evolve.data[0].arg = static_cast<u16>(ItemID::kKingsRock);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kScyther:
    case Species::kOnix:
      evolve.data[0].arg = static_cast<u16>(ItemID::kMetalCoat);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kSeadra:
      evolve.data[0].arg = static_cast<u16>(ItemID::kDragonScale);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kPorygon:
      evolve.data[0].arg = static_cast<u16>(ItemID::kUpGrade);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kPorygon2:
      evolve.data[0].arg = static_cast<u16>(ItemID::kDubiousDisc);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kClamperl:
      evolve.data[0].arg = static_cast<u16>(ItemID::kDeepSeaTooth);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      evolve.data[1].arg = static_cast<u16>(ItemID::kDeepSeaScale);
      evolve.data[1].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kRhydon:
      evolve.data[0].arg = static_cast<u16>(ItemID::kProtector);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kElectabuzz:
      evolve.data[0].arg = static_cast<u16>(ItemID::kElectirizer);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kMagmar:
      evolve.data[0].arg = static_cast<u16>(ItemID::kMagmarizer);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kDusclops:
      evolve.data[0].arg = static_cast<u16>(ItemID::kReaperCloth);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kFeebas:
      evolve.data[0].arg = static_cast<u16>(ItemID::kPrismScale);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kSpritzee:
      evolve.data[0].arg = static_cast<u16>(ItemID::kSachet);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kSwirlix:
      evolve.data[0].arg = static_cast<u16>(ItemID::kWhippedDream);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
    case Species::kPhantump:
    case Species::kPumpkaboo:
      evolve.data[0].arg = static_cast<u16>(ItemID::kLeafStone);
      evolve.data[0].method = static_cast<u16>(EvolutionMethod::kItem);
      break;
  }
}

static void LoadEvolveTableHook(u32 species, u32 b, u32 c, u32 d) {
  HookManager::Call<void>(HookID::kLoadEvolveTable, species, b, c, d);
  PatchEvolve(static_cast<Species>(species));
}

void InitializeEvolveHook() {
  HookManager::Initialize(HookID::kLoadEvolveTable,
                          ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE,
                          (uptr)LoadEvolveTableHook
      );
}
}