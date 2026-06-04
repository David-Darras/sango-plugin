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

#include "ui/plugin_menu.h"

namespace ui {
void LoadDayCareMenu(PluginMenu& menu, void* args) {
  menu.Add("Instant Egg Hatch", CheatCodeId::kInstantEggHatch)
      .Add("Instant Egg Generation", CheatCodeId::kInstantEggGeneration)
      .Add("Instant Max Exp", CheatCodeId::kInstantMaxExpForDayCare);
}
} // namespace ui