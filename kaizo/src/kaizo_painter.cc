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

#include "kaizo/kaizo_painter.h"

#include "system/native/graphics.h"
#include "ui/main_application.h"
#include "core/utils.h"

namespace kaizo {
void Painter::DrawPageBackground(ui::MainApplication& app) {
  sys::Graphics::EnableScissor(301, 10, 90, 130);
  sys::Graphics::FillScreen(1, 0.25, 0, 0.25);
  sys::Graphics::DisableScissor();
}

void Painter::DrawPageItems(ui::MainApplication& app) {
  const u32 cursor = GetCursor(app);
  const u32 offset = GetOffset(app);

  sys::Graphics::EnableScissor(303, 12, 86, 126);

  sys::Graphics::FillScreen(0, 0, 0, 0.5);

  Color blue(1, 0.25, 0, 1);
  Color black(1, 1, 1, 1);

  sys::Graphics::DrawText(391, 15 + 16 * cursor, u">", blue);

  c16 buffer[sys::address::kBufferSize];
  for (u32 i = 0; i < GetDisplayCount(app); i++) {
    GetEntry(app, i + offset).GetDisplayValue(buffer);
    sys::Graphics::DrawText(405, 15 + i * kLineHeight, buffer,
                       cursor == i ? blue : black);
  }

  sys::Graphics::DisableScissor();
}

void Painter::DrawBottomOverlay(sys::Graphics& graphics) {
  static s64 start = 0;
  if (start == 0) {
    core::Utils::GetElapsedTime(&start);
  }
  s64 now;
  core::Utils::GetElapsedTime(&now);
  s64 delta = now - start;
  u32 elapsed_sec = core::Utils::ConvertTimeToSeconds(&delta);
  if (elapsed_sec > 1) return;

  Color background(0, 0, 0, 0.5);
  sys::Graphics::FillScreen(background);
  Color text(1, 0, 0, 1);
  sys::Graphics::SetTextScale(0.6, 0.6);
  sys::Graphics::DrawText(5, 5 + 17 * 0, u"Pokémon Sango Kaizo", text);
  sys::Graphics::DrawText(5, 5 + 17 * 1, u"Version 1.0", text);
  sys::Graphics::DrawText(5, 5 + 17 * 2, u"Created By ZettaD", text);

  sys::Graphics::DrawText(5, 5 + 17 * 4, u"Press [SELECT] to open in-game menu.",
                     text);
}
} // namespace kaizo
