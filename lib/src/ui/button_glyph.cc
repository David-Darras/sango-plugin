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
 * @file button_glyph.cc
 * @brief The symbols of the buttons in the texts of the menu.
 *
 * The declarations are in ui/button_glyph.h.
 */

#include "ui/button_glyph.h"

#include "system/native/font_manager.h"

namespace ui {
namespace {
// The symbols of the buttons: A, B, X, Y, L and R. The font of the game is the
// shared font of the 3DS: it draws the icons of the buttons of Nintendo at
// U+E000 and after. The circled letters of Unicode come next.
struct Glyph {
  c16 letter;
  c16 nintendo;
  c16 circled;
};

const Glyph kGlyphs[] = {
    {u'A', 0xE000, 0x24B6}, {u'B', 0xE001, 0x24B7}, {u'X', 0xE002, 0x24CD},
    {u'Y', 0xE003, 0x24CE}, {u'L', 0xE004, 0x24C1}, {u'R', 0xE005, 0x24C7},
};

// Returns the character of a button: the icon of the font, the circled letter,
// or the letter.
c16 GetGlyph(c16 letter) {
  for (const Glyph& glyph : kGlyphs) {
    if (glyph.letter != letter) continue;
    if (sys::FontManager::IsPrintable(glyph.nintendo)) return glyph.nintendo;
    if (sys::FontManager::IsPrintable(glyph.circled)) return glyph.circled;
    return letter;
  }
  return letter;
}

bool IsMark(const c16* text) {
  if (text[0] != u'{' || text[1] == 0 || text[2] != u'}') return false;
  for (const Glyph& glyph : kGlyphs) {
    if (glyph.letter == text[1]) return true;
  }
  return false;
}
} // namespace

const c16* ExpandButtons(const c16* text, c16* buffer, u32 capacity) {
  // Most texts have no mark: they stay as they are.
  bool has_mark = false;
  for (const c16* c = text; *c != 0; c++) {
    if (IsMark(c)) {
      has_mark = true;
      break;
    }
  }
  if (!has_mark) return text;

  u32 length = 0;
  for (const c16* c = text; *c != 0 && length + 1 < capacity; c++) {
    if (IsMark(c)) {
      buffer[length++] = GetGlyph(c[1]);
      c += 2;
    } else {
      buffer[length++] = *c;
    }
  }
  buffer[length] = 0;
  return buffer;
}
} // namespace ui
