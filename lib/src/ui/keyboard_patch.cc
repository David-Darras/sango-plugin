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

#include "ui/patch/keyboard_patch.h"
#include "core/hook_manager.h"
#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/device.h"
#include "system/native/font_manager.h"
#include "system/native/graphics.h"

namespace ui {

void KeyboardPatch::Initialize() {
  core::HookManager::Initialize(HookId::kKeyboardUpdateKeys,
                          core::address::kKeyboardUpdateKeys,
                          (uptr)UpdateKeys, false);

  // Disables the keyboard's "No Good Word" filter to allow prohibited words,
  // phone numbers, etc.
  WRITE32(address::kKeyboardWordFilter, 0xE3A00000);
  ARM_RET(address::kKeyboardWordFilterReturn);
}

void KeyboardPatch::PatchLoad() {
  MEMORY_SCOPE(sys::address::kMemoryRegionKeyboard, 0x7000);
  core::HookManager::ForceEnable(HookId::kKeyboardUpdateKeys);
  // Force to refresh when pressing L
  ARM_NOP(address::kKeyboardRefreshOnL);
  // Pressing R is like pressing L
  WRITE32(address::kKeyboardKeyHandlerTable + 6 * 4,
          READ32(address::kKeyboardKeyHandlerTable + 4 * 4));
  // Don't switch between the two keyboard mode
  ARM_NOP(address::kKeyboardModeSwitch);

  // No animation
  ARM_NOP(address::kKeyboardAnimation1);
  ARM_NOP(address::kKeyboardAnimation2);
  ARM_NOP(address::kKeyboardAnimation3);
  ARM_NOP(address::kKeyboardAnimation4);

  auto& self = GetInstance();
  self.is_opened = true;
  self.page = 0;
}

void KeyboardPatch::DrawTop() {
  auto& self = GetInstance();
  if (!self.is_opened) return;

  Color color(0, 0, 0, 1);
  sys::Graphics::SetTextScale(0.6, 0.6);
  sys::Graphics::DrawText(10, 10,
                     core::Utils::FormatString(u"Page: %03d/%d", self.page + 1,
                                         kMaxPages)->GetBuffer(),
                     color);
}

u16 KeyboardPatch::AdvancePrintable(u16 current, u32 count) {
  for (u32 i = 0; i < count; i++) {
    do { current++; } while (!sys::FontManager::IsPrintable(current));
  }
  return current;
}

u16 KeyboardPatch::RewindPrintable(u16 current, u32 count) {
  for (u32 i = 0; i < count; i++) {
    if (current == 0) break;
    do { current--; } while (current > 0 && !sys::FontManager::IsPrintable(current));
  }
  return current;
}

void KeyboardPatch::UpdateKeys(uptr self, u32 layout_id, bool is_qwerty,
                          u32 key_count) {
  auto& controller = sys::Controller::GetInstance();
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
  u32 key_index_arg = *(u32*)(b[1] + 24);
  auto calculate_c = reinterpret_cast<u32(*)(u32, u32)>(address::kKeyboardCalculateKeyIndex);

  u32 current_char = offset;
  for (u32 i = 0; i < key_count; i++) {
    while (!sys::FontManager::IsPrintable(current_char)) {
      current_char++;
    }

    u32 c = calculate_c(key_index_arg, i);
    u32 e = base_e + 2 * c;

    *(u16*)(e + 4) = (u16)current_char;
    current_char++;
  }

  return core::HookManager::Call<void>(HookId::kKeyboardUpdateKeys, self, layout_id,
                                 is_qwerty, key_count);
}

} // namespace ui
