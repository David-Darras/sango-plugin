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

#pragma once
#include "common.h"
#include "system/native/core.h"
#include "system/native/font_resource.h"

namespace sys {

class FontManager {
  SINGLETON(FontManager)
public:
  STATIC_INLINE FontManager& GetInstance() {
    return Core::GetInstance().GetFontManager();
  }

  STATIC_INLINE bool IsPrintable(u16 c) {
    return GetInstance().font->HasGlyph(c);
  }

  void* archive;
  Font* font;
  FontResource ctr;
  FontResource game_freak;
  u32 _0;
  FontResource digit;
  FontResource battle;
  FontResource braille;
};

} // namespace sys
