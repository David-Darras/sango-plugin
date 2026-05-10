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

#include "menu/theme.h"
#include "menu/plugin_menu.h"

namespace menu {
void ColorMenu(PluginMenu& menu, void* args) {
  Color& color = *(Color*)args;
  const f32 kFactor = 0.025f;
  menu.Add("Red", color.r).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Green", color.g).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Blue", color.b).WithFactor(kFactor).WithBounds(0, 1)
      .Add("Alpha", color.a).WithFactor(kFactor).WithBounds(0, 1);
}

Theme Theme::instance;

Theme::Theme() {
  background_color = Color{0, 0, 0, 0.75f};
  unselected_text_color = Color{1, 1, 1, 1};
  selected_text_color = Color{0, 1, 0, 1};
  edited_text_color = Color{1, 0, 0, 1};

  open_sound = 7;
  close_sound = 8;
  confirm_sound = 0;
  next_sound = 4;
  error_sound = 21;
}

void Theme::LoadMenu(PluginMenu& menu, void* args) {
  Theme& theme = GetInstance();

  menu.Add("Background Color", ColorMenu, &theme.background_color)
      .Add("Unselected Text Color", ColorMenu, &theme.unselected_text_color)
      .Add("Selected Text Color", ColorMenu, &theme.selected_text_color)
      .Add("Edited Text Color", ColorMenu, &theme.edited_text_color);

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
} // namespace menu