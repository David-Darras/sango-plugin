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
#include "game/constant/evolution_method.h"
#include "game/constant/item.h"
#include "game/constant/species.h"
#include "game/global_data/evolution.h"

namespace feature {
class Evolution {
  MAKE_SINGLETON(Evolution)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadEvolveTable,
                            ADDRESS_GLOBAL_DATA_LOAD_EVOLVE_TABLE,
                            (uptr)LoadEvolveTableHook);
  }

  static void LoadEvolveTableHook(Species species, u32 b, u32 c, u32 d) {
    HookManager::Call<void>(HookID::kLoadEvolveTable, species, b, c, d);
    PatchEvolve(species);
  }

  static void PatchEvolve(Species species) {
    auto& evolve_table = *(global_data::EvolutionTable*)
        READ32(ADDRESS_GLOBAL_DATA_EVOLVE_TABLE);
    auto& table = *evolve_table.data;
    switch (species) {
      case Species::kMachoke:
      case Species::kGraveler:
      case Species::kKadabra:
      case Species::kBoldore:
      case Species::kGurdurr:
      case Species::kHaunter:
      case Species::kKarrablast:
      case Species::kShelmet:
        table.data[0].arg = 30; // level
        table.data[0].method = EvolutionMethod::kLevelUp;
        break;
      case Species::kSlowpoke:
      case Species::kPoliwhirl:
        table.data[0].arg = static_cast<u16>(ItemID::kKingsRock);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kScyther:
      case Species::kOnix:
        table.data[0].arg = static_cast<u16>(ItemID::kMetalCoat);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kSeadra:
        table.data[0].arg = static_cast<u16>(ItemID::kDragonScale);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kPorygon:
        table.data[0].arg = static_cast<u16>(ItemID::kUpGrade);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kPorygon2:
        table.data[0].arg = static_cast<u16>(ItemID::kDubiousDisc);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kClamperl:
        table.data[0].arg = static_cast<u16>(ItemID::kDeepSeaTooth);
        table.data[0].method = EvolutionMethod::kItem;
        table.data[1].arg = static_cast<u16>(ItemID::kDeepSeaScale);
        table.data[1].method = EvolutionMethod::kItem;
        break;
      case Species::kRhydon:
        table.data[0].arg = static_cast<u16>(ItemID::kProtector);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kElectabuzz:
        table.data[0].arg = static_cast<u16>(ItemID::kElectirizer);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kMagmar:
        table.data[0].arg = static_cast<u16>(ItemID::kMagmarizer);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kDusclops:
        table.data[0].arg = static_cast<u16>(ItemID::kReaperCloth);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kFeebas:
        table.data[0].arg = static_cast<u16>(ItemID::kPrismScale);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kSpritzee:
        table.data[0].arg = static_cast<u16>(ItemID::kSachet);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kSwirlix:
        table.data[0].arg = static_cast<u16>(ItemID::kWhippedDream);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      case Species::kPhantump:
      case Species::kPumpkaboo:
        table.data[0].arg = static_cast<u16>(ItemID::kLeafStone);
        table.data[0].method = EvolutionMethod::kItem;
        break;
      default:
        break;
    }
  }
};
}
