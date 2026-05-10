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

#ifndef SANGO_PLUGIN_SAVEDATA_RECORD_H
#define SANGO_PLUGIN_SAVEDATA_RECORD_H
#include "savedata.h"
#include "menu/plugin_menu.h"

namespace savedata {
struct RecordManager {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static FORCE_INLINE RecordManager& GetInstance() {
    return SaveData::GetInstance().GetRecordManager();
  }

  void* vtable;
  u32 records_0[100];
  u16 records_1[100];
  u32 _0;
  bool is_disabled;
};
} // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_RECORD_H