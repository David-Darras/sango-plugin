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

#include "menu/plugin_menu.h"

namespace overworld {
void EnableInstantEggHatch(void*) {
  // Skip check for remaining hatch steps
  // (cf. savedata/pokemon_core_data.h)
  WRITE(u32, 0x00715EF0, 0xEA000007);
}

void EnableInstantEggGeneration(void*) {
  WRITE(u32, 0x00711364, 0xE1A00000);
}

void EnableInstantMaxExp(void*) {
  WRITE(u32, 0x00465A34, 0x15824004);
  WRITE(u32, 0x00465A54, 0x158240F4);
}

void DayCareMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Instant Egg Hatch", EnableInstantEggHatch)
      .Add("Instant Egg Generation", EnableInstantEggGeneration)
      .Add("Instant Max Exp", EnableInstantMaxExp);
}
} // namespace overworld