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
#include "ui/log_application.h"

namespace renderer {
struct TextBox;
}

class AppLayoutSubManager {
  SINGLETON(AppLayoutSubManager)
public:
};

class AppLayoutManager {
  SINGLETON(AppLayoutManager)
public:
  INLINE uptr GetSubManager() {
    return READ(uptr, (uptr)this + 4);
  }

  INLINE uptr GetToken(u32 layout_id) {
    return *(uptr*)(*(uptr*)((uptr)this + 28) + 8 * layout_id);
  }

  INLINE renderer::TextBox* GetTextBox(u32 layout_id, u32 pane_id) {
    return ((renderer::TextBox*(*)(uptr, u32))
      0x004C8CE0)(GetToken(layout_id), pane_id);
  }

  void
  SetTextBoxStringValue(u32 layout_id, u32 pane_id, const c16* str, ...) {
    va_list args;
    va_start(args, str);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        String::GetTmpBuf(), BUFFER_SIZE, str, args);
    va_end(args);

    ((void(*)(renderer::TextBox*, String*))0x4195F4)(
        GetTextBox(layout_id, pane_id), String::GetTmpStr());
  }

  INLINE void SetTextBoxIntegerValue(u32 layout_id, u32 pane_id, u32 text_id,
                                     u32 value,
                                     u32 digit_count) {
    return ((void(*)(AppLayoutManager*, u32, u32, u32, u32, u32, u32, u32, u32))
      ADDRESS_APP_LAYOUT_MANAGER_SET_TEXT_BOX_INTEGER_VALUE)(
        this, layout_id, pane_id, text_id, value, digit_count, 0, 0, 1
        );
  }

  INLINE void SetTextBoxColor(u32 layout_id, u32 pane_id, Color8* top_color,
                              Color8* bottom_color) {
    return ((void(*)(AppLayoutManager*, u32, u32, Color8*, Color8*))
      ADDRESS_APP_LAYOUT_MANAGER_SET_TEXT_BOX_COLOR)(
        this, layout_id, pane_id, top_color, bottom_color
        );
  }

  INLINE void Show(u32 layout_id, u32 pane_id) {
    return ((void(*)(AppLayoutManager*, u32, u32, bool))
      ADDRESS_APP_LAYOUT_MANAGER_SHOW_PANE)(
        this, layout_id, pane_id, true
        );
  }

  INLINE void Hide(u32 layout_id, u32 pane_id) {
    return ((void(*)(AppLayoutManager*, u32, u32, bool))
      ADDRESS_APP_LAYOUT_MANAGER_SHOW_PANE)(
        this, layout_id, pane_id, false
        );
  }
};