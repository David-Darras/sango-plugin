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

/**
 * @file page_theme.cc
 * @brief The menu page of the theme of the plugin.
 */

#include "ui/main_application.h"
#include "ui/page/page_common.h"

namespace ui {
void LoadThemePage(MainApplication& app, void* args) {
  auto& theme = Theme::GetInstance();

  app.AddSection("Ready-Made Looks")
      .Add("Default", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kDefault);
      })
      .WithDescription("The colors change at once. The menu saves them in "
                       "sdmc:/sango/menu.ini when it closes.")
      .Add("Dark", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kDark);
      })
      .Add("Light", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kLight);
      })
      .Add("Ruby", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kRuby);
      })
      .Add("Sapphire", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kSapphire);
      })
      .Add("Emerald", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kEmerald);
      });

  app.AddSection("Effects")
      .Add("Text Shadow", theme.text_shadow)
      .WithDescription("A shadow under the texts of the top screen, on the "
                       "pages that show the game (camera, models).")
      .Add("Animations", theme.animations)
      .WithDescription("The selection bar and the pages move smoothly. Off: "
                       "they move at once.");

  app.AddSection("Colors")
      .Add("Background Color", LoadColorPage, &theme.background_color)
      .Add("Unselected Text Color", LoadColorPage,
           &theme.unselected_text_color)
      .Add("Selected Text Color", LoadColorPage, &theme.selected_text_color);

  app.AddSection("Sounds")
      .Add("Open Plugin Sound Effect", theme.open_sound)
      .Add("Close Plugin Sound Effect", theme.close_sound)
      .Add("Confirm Sound Effect", theme.confirm_sound)
      .Add("Next Sound Effect", theme.next_sound)
      .Add("Error Sound Effect", theme.error_sound);

  static const char* KEYS[] = {
      "None", "Left", "Right", "Up", "Down", "A", "B", "X", "Y", "L", "R",
      "ZL", "ZR", "Start/Select"
  };

  app.AddSection("Menu Button")
      .Add("Key 1", theme.keys[0])
      .WithDescription("The buttons that open the menu. None for all three: "
                       "START. ZL and ZR do not exist on a standard 3DS.")
      .WithArray(KEYS, SIZE(KEYS))
      .Add("Key 2", theme.keys[1])
      .WithArray(KEYS, SIZE(KEYS))
      .Add("Key 3", theme.keys[2])
      .WithArray(KEYS, SIZE(KEYS));
}
} // namespace ui