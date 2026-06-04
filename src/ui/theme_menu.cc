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
#include "ui/common_menu.h"

namespace ui {
void LoadThemeMenu(menu::PluginMenu& menu, void* args) {
  auto& theme = menu::Theme::GetInstance();

  menu.Add("Background Color", LoadColorMenu, &theme.background_color)
      .Add("Unselected Text Color", LoadColorMenu,
           &theme.unselected_text_color)
      .Add("Selected Text Color", LoadColorMenu, &theme.selected_text_color)
      .Add("Edited Text Color", LoadColorMenu, &theme.edited_text_color);

  menu.AddSeparator()
      .Add("Open Plugin Sound Effect", theme.open_sound)
      .Add("Close Plugin Sound Effect", theme.close_sound)
      .Add("Confirm Sound Effect", theme.confirm_sound)
      .Add("Next Sound Effect", theme.next_sound)
      .Add("Error Sound Effect", theme.error_sound);

  static const char* KEYS[] = {
      "None", "Left", "Right", "Up", "Down", "A", "B", "X", "Y", "L", "R",
      "ZL", "ZR", "Start/Select"
  };

  menu.AddSeparator()
      .Add("Key 1", theme.keys[0])
      .WithArray(KEYS, SIZE(KEYS))
      .Add("Key 2", theme.keys[1])
      .WithArray(KEYS, SIZE(KEYS))
      .Add("Key 3", theme.keys[2])
      .WithArray(KEYS, SIZE(KEYS));
}
} // namespace ui