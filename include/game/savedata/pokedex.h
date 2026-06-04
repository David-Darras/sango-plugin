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
#include "game/savedata/savedata.h"
#include "game/data_manager.h"

namespace savedata {
struct Pokedex {
  STATIC_INLINE Pokedex& GetInstance() {
    return SaveData::GetInstance().GetPokedex();
  }

  /**
* @return Form index, or -1 if the species has no alternative forms.
*/
  INLINE s32 GetFormIndex(u16 species) {
    return ((s32(*)(Pokedex*, u16))ADDRESS_POKEDEX_GET_FORM_INDEX)(
        this, species);
  }

  static void GetTableIndexAndFormMax(s32& table_idx, s32& form_max,
                                      u16 species) {
    struct Entry {
      u16 species;
      u16 form_max;
      bool is_mega_evolution;
    };

    Entry* entry = (Entry*)ADDRESS_POKEDEX_FORM_TABLE;
    table_idx = 0;
    form_max = -1;
    while (entry->species != 0) {
      if (entry->species == species) {
        form_max = entry->form_max;
        return;
      }
      table_idx++;
      entry++;
    }
  }

  void* vtable;
  u32 signature;
  u32 _0;
  u32 captured_flags[24];
  u32 gender_seen_flags[4][24];
  u32 displayed_gender_flags[4][24];
  u8 form_seen_flags[2][38];
  u8 displayed_form_flags[2][38];
  u8 language_version_flags[640];
  u32 spinda_pattern;
  u16 seen_count[722];
  u16 captured_count[722]; // unused
};
} // namespace savedata