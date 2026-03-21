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

#include "menu/numpad.h"

#include <string.h>

#include "system/sound.h"

namespace menu {

Numpad::Numpad() : cursor_(0) {
  memset(input_, 0, sizeof(input_));

  constexpr s32 x = 10;
  constexpr s32 y_start = 10;
  constexpr s32 width = 30;
  constexpr s32 height = 30;
  constexpr s32 bar_width = 300;
  constexpr s32 bar_height = 18;
  constexpr s32 btn_width = 100;
  constexpr s32 btn_height = 18;

  // Input display bar
  buttons_[kButtonInput].Initialize(x, y_start, bar_width, bar_height);

  // Numeric buttons 0-9
  s32 current_y = y_start + bar_height - 1;
  for (int i = 0; i < 10; ++i) {
    buttons_[kButton0 + i].Initialize(x + i * width, current_y, width, height);
  }

  // Control buttons
  current_y += height - 1;
  buttons_[kButtonCancel].Initialize(x, current_y, btn_width, btn_height);
  buttons_[kButtonDelete].Initialize(x + btn_width, current_y, btn_width,
                                     btn_height);
  buttons_[kButtonOk].Initialize(x + btn_width * 2, current_y, btn_width,
                                 btn_height);
}

void Numpad::Draw() const {
  c16 buffer[2] = {0, 0};

  // Render text input
  buttons_[kButtonInput].Draw(input_, 2, 0);

  // Render numbers
  for (int i = 0; i < 10; ++i) {
    buffer[0] = u'0' + i;
    buttons_[kButton0 + i].Draw(buffer, 10, 6);
  }

  // Render controls
  buttons_[kButtonCancel].Draw(u"CANCEL", 19, 0);
  buttons_[kButtonDelete].Draw(u"DELETE", 20, 0);
  buttons_[kButtonOk].Draw(u"OK", 39, 0);
}

void Numpad::Update() {
  for (u32 i = 0; i < kButtonMax; ++i) {
    buttons_[i].Update();
  }

  // Check digits
  for (u32 i = 0; i <= 9; ++i) {
    if (buttons_[kButton0 + i].IsReleased()) {
      Sound::PlaySoundEffect(4);
      AddDigit(i);
    }
  }

  // Check controls
  if (buttons_[kButtonDelete].IsReleased()) {
    Sound::PlaySoundEffect(4);
    RemoveLastDigit();
  }

  if (buttons_[kButtonCancel].IsReleased()) {
    Sound::PlaySoundEffect(4);
    cursor_ = 0;
    memset(input_, 0, sizeof(input_));
  }
}

bool Numpad::IsButtonOkReleased() const {
  return buttons_[kButtonOk].IsReleased();
}

u32 Numpad::GetInput() const { return UnicodeToInteger(input_); }

void Numpad::AddDigit(u32 digit) {
  if (cursor_ >= 12) return;
  input_[cursor_] = u'0' + digit;
  cursor_++;
  input_[cursor_] = 0;
}

void Numpad::RemoveLastDigit() {
  if (cursor_ <= 0) return;
  cursor_--;
  input_[cursor_] = 0;
}

u32 Numpad::UnicodeToInteger(const c16 *str) {
  if (*str == 0) {
    return 0;
  }

  u32 result = 0;
  u32 base = 10;
  const c16 *current = str;

  // Check for hex prefix
  if (current[0] == u'0' && (current[1] == u'x' || current[1] == u'X')) {
    base = 16;
    current += 2;
  }

  while (*current != 0) {
    u32 digit_value;
    c16 c = *current;

    if (c >= u'0' && c <= u'9') {
      digit_value = c - u'0';
    } else if (base == 16 && c >= u'A' && c <= u'F') {
      digit_value = 10 + (c - u'A');
    } else if (base == 16 && c >= u'a' && c <= u'f') {
      digit_value = 10 + (c - u'a');
    } else {
      break;
    }

    result = result * base + digit_value;
    current++;
  }

  return result;
}

}  // namespace menu