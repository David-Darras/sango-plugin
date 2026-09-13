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

namespace ui {

class KeyboardPatch {
  MAKE_SINGLETON(KeyboardPatch)

public:
  bool is_opened = false;
  u8 page = 0;

  static constexpr u8 kMaxPages = 154;

  static void Initialize();
  static void PatchLoad();
  static void DrawTop();

  static void UpdateKeys(uptr self, u32 layout_id, bool is_qwerty,
                         u32 key_count);

private:
  static u16 AdvancePrintable(u16 current, u32 count);
  static u16 RewindPrintable(u16 current, u32 count);
};

} // namespace ui
