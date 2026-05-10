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

#include "savedata/record_manager.h"
#include "menu/plugin_menu.h"

#define ADDRESS_RECORD_MAX_VALUE_TABLE (0x0058DE14) // 999999999, 9999999, etc.
#define ADDRESS_RECORD_MAX_VALUE_INDEX_TABLE (0x0058DD4A) // 0, 0, 0, 1, 1, etc.

namespace savedata {
#include "savedata/records.inc"

void RecordManager::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u32 record_0_idx = 0;
  static u32 record_1_idx = 0;

  RecordManager& man = RecordManager::GetInstance();

  menu.Add("Is Disabled", man.is_disabled)
      .Add("Record 0", record_0_idx)
      .WithArray(RECORDS_0, SIZE(RECORDS_0))
      .WithRefresh()
      .Add("Value", man.records_0[record_0_idx])
      .Add("Record 1", record_1_idx)
      .WithArray(RECORDS_1, SIZE(RECORDS_1))
      .WithRefresh()
      .Add("Value", man.records_1[record_1_idx]);
}
} // namespace savedata