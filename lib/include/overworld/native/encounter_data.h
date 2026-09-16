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

#include "core/types.h"
#include "overworld/constant/encounter_method.h"
#include "overworld/native/poke_info_on_action.h"

namespace overworld {

struct EncounterData {
  u8 rate[14];

  union {
    PokeInfoOnAction poke_info[61];

    struct {
      PokeInfoOnAction on_walk[12];
      PokeInfoOnAction on_xxx[12];
      PokeInfoOnAction on_yyy[3];
      PokeInfoOnAction on_surf[5];
      PokeInfoOnAction on_rock_smash[5];
      PokeInfoOnAction on_old_rod[3];
      PokeInfoOnAction on_good_rod[3];
      PokeInfoOnAction on_super_rod[3];
      PokeInfoOnAction on_horde[3 * 5];
    };
  };

  INLINE u8& GetRate(EncounterMethod action) {
    return rate[static_cast<u8>(action)];
  }

  INLINE PokeInfoOnAction*
  GetPokeInfoTable(EncounterMethod action, u32& count) {
    PokeInfoOnAction* output = nullptr;
    count = 0;
    switch (action) {
      case EncounterMethod::kWalk:
        if (GetRate(EncounterMethod::kWalk) > 0) {
          output = on_walk;
          count = 12;
        }
        break;
    }
    return output;
  }
};

} // namespace overworld
