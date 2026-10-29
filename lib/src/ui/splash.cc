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
 * @file splash.cc
 * @brief The splash screen of the plugin.
 *
 * The declarations are in ui/splash.h.
 */

#include "ui/splash.h"

#include <cstring>

#include "core/utils.h"
#include "system/native/graphics.h"
#include "ui/theme.h"

namespace ui {
namespace {
// The last frames fade out.
constexpr u32 kFadeFrames = 30;

// Draws a text at the center of the top screen. The texts of the top screen
// use a space of 512 x 256.
void DrawCentered(s32 y, f32 scale, const c16* text, Color color) {
  sys::Graphics::SetTextScale(scale, scale);
  const s32 x = (512 - sys::Graphics::GetTextWidth(text)) / 2;
  sys::Graphics::DrawTextWithShadow(x, y, text, color);
}
} // namespace

void Splash::DrawTop() {
  static u32 frame = 0;
  if (frame >= kFrames) return;
  frame++;

  f32 fade = 1.0f;
  if (kFrames - frame < kFadeFrames) {
    fade = static_cast<f32>(kFrames - frame) / kFadeFrames;
  }

  const Theme& theme = Theme::GetInstance();
  sys::Graphics::FillScreen(Color(0, 0, 0, 0.7f * fade));

  Color title = theme.selected_text_color;
  title.a *= fade;
  Color text = theme.unselected_text_color;
  text.a *= fade;

  // The name in Japanese, small, above the name of the plugin: Sango (ORAS)
  // or Kujira (XY).
#ifdef GAME_XY
  static const c16 kJapaneseName[] = {0x300E, 0x304F, 0x3058, 0x3089, 0x300F, 0};
#else
  static const c16 kJapaneseName[] = {0x300E, 0x3055, 0x3093, 0x3054, 0x300F, 0};
#endif
  DrawCentered(52, 0.6f, kJapaneseName, text);

  c16 line[96];
  core::Utils::Format(line, u"%s Plugin", PLUGIN_NAME);
  DrawCentered(80, 1.1f, line, title);
  core::Utils::Format(line, u"Version %s", PLUGIN_VERSION);
  DrawCentered(125, 0.6f, line, text);
  core::Utils::Format(line, u"Build %s  %s", __DATE__, __TIME__);
  DrawCentered(150, 0.5f, line, text);
  core::Utils::Format(line, u"by %s", PLUGIN_CREATOR);
  DrawCentered(175, 0.6f, line, title);

  // The buttons that open the menu (START when no key is set).
  c8 keys[64] = "";
  const c8** names = GetKeyNames();
  for (u32 i = 0; i < SIZE(theme.keys) && theme.keys[i] != 0; i++) {
    if (theme.keys[i] >= kKeyNameCount) continue;
    if (keys[0] != '\0') strncat(keys, " + ", sizeof(keys) - strlen(keys) - 1);
    strncat(keys, names[theme.keys[i]], sizeof(keys) - strlen(keys) - 1);
  }
  if (keys[0] == '\0') strcpy(keys, "START");
  core::Utils::Format(line, u"Press %s to open the menu", keys);
  DrawCentered(205, 0.6f, line, text);
}
} // namespace ui
