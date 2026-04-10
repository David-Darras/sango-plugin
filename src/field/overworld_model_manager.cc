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

#include "field/overworld_model_manager.h"

#include "menu/plugin_menu.h"

namespace field {

void OverworldModelManager::LoadMenu(menu::PluginMenu& menu, void* args) {
  OverworldModelManager& man = GetInstance();
  OverworldModelResource& rsrc = man.GetResource(0);
  menu.Add("Player Model Id", rsrc.model_id);
}

}  // namespace field
