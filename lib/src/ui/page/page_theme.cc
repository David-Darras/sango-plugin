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
      })
      .Add("High Contrast", [](void*) {
        Theme::GetInstance().ApplyPreset(Theme::Preset::kHighContrast);
      })
      .WithDescription("Black, white and yellow: the texts are easy to "
                       "read. Turn off the background image too.");

  app.AddSection("Effects")
      .Add("Text Shadow", theme.text_shadow)
      .WithDescription("A shadow under the texts. On a page with many texts, "
                       "some texts have no shadow: the GPU has a limit.")
      .Add("Background Image", theme.background_image)
      .WithDescription("Shows sdmc:/sango/menu_top.tga and menu_bottom.tga "
                       "(TGA, 400 x 240 and 320 x 240) behind the menu.")
      .Add("Image Opacity", theme.background_image_opacity)
      .WithFactor(0.1f)
      .WithBounds(0, 1)
      .WithDescription("0: no image. 1: the full image. The background color "
                       "covers the image too: lower its alpha.")
      .Add("Animations", theme.animations)
      .WithDescription("The selection bar and the pages move smoothly. Off: "
                       "they move at once.");

  app.AddSection("Controls")
      .Add("Menu Sounds", theme.sounds)
      .WithDescription("Off: the menu plays no sound.")
      .Add("Fast Scroll Delay", theme.fast_delay)
      .WithBounds(5, 120)
      .WithDescription("The frames (60 = 1 second) that a held button waits "
                       "before the menu moves faster.");

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