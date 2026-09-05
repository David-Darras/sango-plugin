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
#include "game/constant/item.h"
#include "game/global_data/item.h"
#include "game/global_data/mega_evolution.h"

#include "kaizo/kaizo.h"

namespace feature {
class MegaEvolution {
  MAKE_SINGLETON(MegaEvolution)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGlobalDataLoadMegaEvolutionTable,
                            ADDRESS_GLOBAL_DATA_LOAD_MEGA_EVOLUTION_TABLE,
                            (uptr)LoadMegaEvolutionTableHook);
    // disable cache
    ARM_RET(ADDRESS_GLOBAL_DATA_LOAD_MEGA_EVOLUTION_TABLE + 0x3C);
  }

  static void LoadMegaEvolutionTableHook(Species species) {
    HookManager::Call<void>(HookID::kGlobalDataLoadMegaEvolutionTable, species);

    auto& mega_evolve_table = *(global_data::MegaEvolutionTable*)
        READ32(ADDRESS_GLOBAL_DATA_MEGA_EVOLUTION_TABLE);
    auto& table = *mega_evolve_table.data;
    if (species == Species::kWailord) {
      table.entry[0].form = Form::kMega;
      table.entry[0].method = MegaEvolutionMethod::kItem;
      table.entry[0].item = ItemID::kMeteorite3;
      ui::LogApplication::Print(u"Mega-Wailord");
    }
    if (species == Species::kVenusaur) {
      table.entry[0].form = Form::kVenusaurNormal;
      table.entry[0].method = MegaEvolutionMethod::kNone;
      table.entry[0].item = ItemID::kNone;
    }
    ui::LogApplication::Print(u"%u %u %u %u", species, table.entry[0].form,
                              table.entry[0].method, table.entry[0].item);
  }
};
}