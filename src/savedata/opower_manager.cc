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

#include "savedata/opower_manager.h"
#include "menu/plugin_menu.h"

namespace savedata {
#include "savedata/opower.inc"

void OPowerManager::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u32 learned_opower_idx = 0;
  static u32 field_opower_idx = 0;
  static u32 battle_opower_idx = 0;

  OPowerManager& man = GetInstance();

  menu.Add("OPower Points", man.power_points)
      .Add("Learned OPower Index", learned_opower_idx)
      .WithArray(OPOWERS, SIZE(OPOWERS))
      .WithRefresh()
      .Add("Learned OPower Value", man.learned_powers[learned_opower_idx])
      .AddSeparator()
      .Add("Field OPower Index", field_opower_idx)
      .WithArray(FIELD_OPOWERS, SIZE(FIELD_OPOWERS))
      .WithRefresh()
      .Add("Field Lv. 1 Uses", man.field_power_level_1_uses[field_opower_idx])
      .Add("Field Lv. 2 Uses", man.field_power_level_2_uses[field_opower_idx])
      .AddSeparator()
      .Add("Battle OPower Index", battle_opower_idx)
      .WithArray(BATTLE_OPOWERS, SIZE(BATTLE_OPOWERS))
      .WithRefresh()
      .Add("Battle Lv. 1 Uses",
           man.battle_power_level_1_uses[battle_opower_idx])
      .Add("Battle Lv. 2 Uses",
           man.battle_power_level_2_uses[battle_opower_idx]);
}
} // namespace savedata