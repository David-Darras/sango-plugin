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
#include "hook_manager.h"
#include "utils.h"
#include "system/device.h"
#include "system/font_manager.h"
#include "system/graphics.h"

namespace feature {
class Keyboard {
  MAKE_SINGLETON(Keyboard)
public:
  bool is_opened = false;
  u8 page = 0;

  static constexpr u8 kMaxPages = 154;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kKeyboardUpdateKeys,
                            ADDRESS_KEYBOARD_UPDATE_KEYS,
                            (uptr)UpdateKeys, false);
  }

  STATIC_INLINE void PatchLoad() {
    HookManager::ForceEnable(HookID::kKeyboardUpdateKeys);
    // Force to refresh when pressing L
    SAFE_ARM_NOP(0x0074323C);
    // Pressing R is like pressing L
    SAFE_WRITE32(0x00742F58 + 6 * 4, 0x00742F58 + 4 * 4);
    // Don't switch between the two keyboard mode
    SAFE_ARM_NOP(0x00746294);
    auto& self = GetInstance();
    self.is_opened = true;
    self.page = 0;
  }

  STATIC_INLINE void DrawTop() {
    auto& self = GetInstance();
    if (!self.is_opened) return;

    Color color(0, 0, 0, 1);
    Graphics::SetTextScale(0.6, 0.6);
    Graphics::DrawText(10, 10,
                       Utils::FormatString(u"Page: %03d/%d", self.page + 1,
                                           kMaxPages)->GetBuffer(),
                       color);
  }

private:
  static u16 AdvancePrintable(u16 current, u32 count) {
    for (u32 i = 0; i < count; i++) {
      do { current++; } while (!FontManager::IsPrintable(current));
    }
    return current;
  }

  static u16 RewindPrintable(u16 current, u32 count) {
    for (u32 i = 0; i < count; i++) {
      if (current == 0) break;
      do { current--; } while (current > 0 && !
                               FontManager::IsPrintable(current));
    }
    return current;
  }

public:
  static void UpdateKeys(uptr self, u32 layout_id, bool is_qwerty,
                         u32 key_count) {
    auto& controller = Controller::GetInstance();
    auto& kb = GetInstance();
    static u16 offset = 0;

    if (controller.IsKeyPressed(Key::kR)) {
      kb.page++;
      if (kb.page >= kMaxPages) {
        kb.page = 0;
        offset = 0;
      } else {
        offset = AdvancePrintable(offset, key_count);
      }
    } else if (controller.IsKeyPressed(Key::kL)) {
      if (kb.page == 0) {
        kb.page = kMaxPages - 1;
        offset = AdvancePrintable(0, kb.page * key_count);
      } else {
        kb.page--;
        offset = RewindPrintable(offset, key_count);
      }
    }

    u32* a = (u32*)self;
    u32* b = *(u32**)(self + 100);

    u32 d = a[20];
    if (is_qwerty) {
      d = (a[24] ? 6 : 4) + a[21];
    }

    u32 base_e = *(u32*)(*(u32*)(a[25] + 8) + 4 * d);
    u32 arg_4C5924 = *(u32*)(b[1] + 24);
    auto calculate_c = reinterpret_cast<u32(*)(u32, u32)>(0x4C5924);

    u32 current_char = offset;
    for (u32 i = 0; i < key_count; i++) {
      while (!FontManager::IsPrintable(current_char)) {
        current_char++;
      }

      u32 c = calculate_c(arg_4C5924, i);
      u32 e = base_e + 2 * c;

      *(u16*)(e + 4) = (u16)current_char;
      current_char++;
    }

    return HookManager::Call<void>(HookID::kKeyboardUpdateKeys, self, layout_id,
                                   is_qwerty, key_count);
  }
};
}