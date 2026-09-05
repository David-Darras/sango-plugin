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
#include "game/core/data_manager.h"

namespace overworld {
struct PokeInfoOnAction {
  u16 species : 11;
  u16 form : 5;
  u8 min_level;
  u8 max_level;
};

enum EncounterAction {
  kWalk, kXXX, kYYY, kSurf, kRockSmash,
  kOldRod, kGoodRod, kSuperRod, kHorde,
  kMax
};

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

  INLINE PokeInfoOnAction*
  GetPokeInfoTable(EncounterAction action, u32& count) {
    PokeInfoOnAction* output = nullptr;
    count = 0;
    switch (action) {
      case EncounterAction::kWalk:
        if (rate[EncounterAction::kWalk] > 0) {
          output = on_walk;
          count = 12;
        }
        break;
    }
    return output;
  }
};

struct MapData {
  SINGLETON(MapData)
  STATIC_INLINE MapData& GetInstance() {
    return game::DataManager::GetInstance().GetMapData();
  }

  INLINE EncounterData& GetEncounterData() {
    return *(EncounterData*)bundle[choice]->GetResource(3);
  }

  u32 _0;
  void* archive;
  void* _1;
  u16 map_id;
  u16 next_map_id;

  u8 buffer[2][0x4A58];
  Bundle* bundle[2];
  u8 buffer2[0x4A58];
  u8 choice;

  u16 map_name[64];
};
} // namespace overworld