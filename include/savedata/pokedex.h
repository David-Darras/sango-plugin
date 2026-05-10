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

#ifndef SANGO_PLUGIN_SAVEDATA_POKEDEX_H
#define SANGO_PLUGIN_SAVEDATA_POKEDEX_H
#include "savedata.h"
#include "menu/plugin_menu.h"
#include "core/game_data_manager.h"

#define ADDRESS_POKEDEX_FORM_TABLE (0x0058C4A4)
#define ADDRESS_POKEDEX_GET_FORM_INDEX (0x004EB37C)

namespace savedata {
struct Pokedex {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static FORCE_INLINE Pokedex& GetInstance() {
    return SaveData::GetInstance().GetPokedex();
  }

  /**
   * @return Form index, or -1 if the species has no alternative forms.
   */
  FORCE_INLINE s32 GetFormIndex(u16 species) {
    return ((s32(*)(Pokedex*, u16))ADDRESS_POKEDEX_GET_FORM_INDEX)(
        this, species);
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

#endif  // SANGO_PLUGIN_SAVEDATA_POKEDEX_H