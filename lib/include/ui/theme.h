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
 * @file theme.h
 * @brief The colors, the sounds and the buttons of the menu.
 */

#pragma once

#include "common.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"

namespace ui {
/// The colors, the sounds and the buttons of the menu. The page Plugin Theme changes them.
struct Theme {
  MAKE_SINGLETON(Theme)
  Color background_color = Color{0, 0, 0, 0.75f};
  Color unselected_text_color = Color{1, 1, 1, 1};
  Color selected_text_color = Color{1, 0.1f, 0.5f, 1};

  u16 open_sound = 7;
  u16 close_sound = 8;
  u16 confirm_sound = 1;
  u16 next_sound = 4;
  u16 error_sound = 21;

  u8 keys[3] = {};

  bool text_shadow = true; ///< Draws a shadow under the texts of the menu.
  bool animations = true; ///< Moves the cursor and the pages smoothly.
  /// Draws sdmc:/sango/menu_top.tga and menu_bottom.tga behind the menu.
  bool background_image = true;
  /// The opacity of the background images: 0 (hidden) to 1 (opaque).
  f32 background_image_opacity = 1.0f;
  bool sounds = true; ///< Plays the sounds of the menu.
  /// The frames that Up / Down or Left / Right stay pressed before the menu
  /// moves faster.
  u8 fast_delay = 45;

  /// Plays a sound of the menu, when Theme::sounds is on.
  void Play(u16 sound) const {
    if (sounds) sys::Sound::PlaySoundEffect(sound);
  }

  /// The ready-made looks of the Plugin Theme page.
  enum class Preset : u8 {
    kDefault,
    kDark,
    kLight,
    kRuby,
    kSapphire,
    kEmerald,
    kHighContrast, ///< Black, white and yellow: easy to read.
  };

  /// Sets the three colors of a ready-made look.
  void ApplyPreset(Preset preset) {
    switch (preset) {
      case Preset::kDefault:
        SetColors(Color{0, 0, 0, 0.75f}, Color{1, 1, 1, 1},
                  Color{1, 0.1f, 0.5f, 1});
        break;
      case Preset::kDark:
        SetColors(Color{0.05f, 0.05f, 0.08f, 0.92f},
                  Color{0.85f, 0.85f, 0.9f, 1}, Color{0.4f, 0.8f, 1, 1});
        break;
      case Preset::kLight:
        SetColors(Color{0.95f, 0.95f, 0.92f, 0.92f},
                  Color{0.15f, 0.15f, 0.2f, 1}, Color{0.85f, 0.2f, 0.1f, 1});
        break;
      case Preset::kRuby:
        SetColors(Color{0.25f, 0.02f, 0.05f, 0.85f}, Color{1, 0.9f, 0.9f, 1},
                  Color{1, 0.35f, 0.3f, 1});
        break;
      case Preset::kSapphire:
        SetColors(Color{0.02f, 0.06f, 0.25f, 0.85f},
                  Color{0.9f, 0.93f, 1, 1}, Color{0.35f, 0.7f, 1, 1});
        break;
      case Preset::kEmerald:
        SetColors(Color{0.02f, 0.18f, 0.08f, 0.85f},
                  Color{0.9f, 1, 0.92f, 1}, Color{0.4f, 1, 0.55f, 1});
        break;
      case Preset::kHighContrast:
        SetColors(Color{0, 0, 0, 0.97f}, Color{1, 1, 1, 1},
                  Color{1, 0.9f, 0, 1});
        break;
    }
  }

private:
  void SetColors(Color background, Color text, Color selected) {
    background_color = background;
    unselected_text_color = text;
    selected_text_color = selected;
  }
};
/// Draws a text of the menu, with a shadow when Theme::text_shadow is on.
inline void DrawMenuText(s32 x, s32 y, const c16* text, Color color) {
  if (Theme::GetInstance().text_shadow) {
    sys::Graphics::DrawTextWithShadow(x, y, text, color);
  } else {
    sys::Graphics::DrawText(x, y, text, color);
  }
}
} // namespace ui