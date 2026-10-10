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
 * @file numpad.cc
 * @brief The numpad of the menu on the bottom screen.
 *
 * The declarations are in ui/widget/numpad.h.
 */

#include "ui/widget/numpad.h"

#include <string.h>

#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "ui/theme.h"

namespace ui {
namespace {
constexpr s32 kKeyWidth = 52;
constexpr s32 kKeyHeight = 25;
constexpr s32 kGap = 2;
constexpr s32 kBarHeight = 20;
constexpr u16 kKeySound = 4;

// The keys of the grid, from the top-left corner: the layout of a phone.
const u8 kGridKeys[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 11};

// The hexadecimal mode: four columns. The digits 1 to 9 stay in the layout
// of a phone, and A to F are on the right and at the bottom.
constexpr u32 kHexColumns = 4;
constexpr s32 kHexKeyWidth = (Numpad::kKeysWidth - 3 * kGap) / 4;
const u8 kHexKeys[16] = {1, 2, 3, 16, 4, 5, 6, 17, 7, 8, 9, 18, 0, 19, 20, 21};

// The characters of the keys of kHexKeys (the ButtonId of A is 16).
c16 GetKeyCharacter(u32 key) {
  return key <= 9 ? (c16)(u'0' + key) : (c16)(u'A' + (key - 16));
}
} // namespace

Numpad::Numpad()
  : cursor_(0), is_edited_(false), allow_minus_(false), allow_dot_(false) {
  memset(input_, 0, sizeof(input_));
  Initialize(10, 10);
}

void Numpad::Initialize(s32 x, s32 y) {
  x_ = x;
  y_ = y;
  buttons_[kButtonInput].Initialize(x, y, kKeysWidth, kBarHeight);
  PlaceKeys();

  const s32 keys_y = y + kBarHeight + 4;

  // DEL, CLR and OK are on the right of the keys, in the two last rows.
  const s32 right_x = x + kKeysWidth + 6;
  const s32 right_width = 300 - kKeysWidth - 6;
  const s32 half = (right_width - kGap) / 2;
  buttons_[kButtonDelete].Initialize(right_x, keys_y + 2 * kRowHeight, half,
                                     kKeyHeight);
  buttons_[kButtonClear].Initialize(right_x + half + kGap,
                                    keys_y + 2 * kRowHeight, half, kKeyHeight);
  buttons_[kButtonOk].Initialize(right_x, keys_y + 3 * kRowHeight, right_width,
                                 kKeyHeight);
}

void Numpad::PlaceKeys() {
  const s32 keys_y = y_ + kBarHeight + 4;
  if (is_hex_) {
    for (u32 i = 0; i < SIZE(kHexKeys); i++) {
      const s32 column = i % kHexColumns;
      const s32 row = i / kHexColumns;
      buttons_[kHexKeys[i]].Initialize(x_ + column * (kHexKeyWidth + kGap),
                                       keys_y + row * kRowHeight,
                                       kHexKeyWidth, kKeyHeight);
    }
    return;
  }
  for (u32 i = 0; i < SIZE(kGridKeys); i++) {
    const s32 column = i % 3;
    const s32 row = i / 3;
    buttons_[kGridKeys[i]].Initialize(x_ + column * (kKeyWidth + kGap),
                                      keys_y + row * kRowHeight, kKeyWidth,
                                      kKeyHeight);
  }
}

void Numpad::SetHex(bool is_hex) {
  if (is_hex_ == is_hex) return;
  is_hex_ = is_hex;
  PlaceKeys();
  for (u32 i = 0; i < kButtonMax; i++) buttons_[i].Reset();
}

void Numpad::DrawKey(ButtonId id, const c16* label, bool is_enabled) const {
  const Theme& theme = Theme::GetInstance();
  const Button& button = buttons_[id];
  const s32 x = button.GetX();
  const s32 y = button.GetY();
  const s32 width = button.GetWidth();
  const s32 height = button.GetHeight();

  // One rectangle for each key: see the comment at the start of
  // main_application.cc.
  Color border = theme.unselected_text_color;
  if (!is_enabled) border.a = 0.3f;
  const bool is_down = is_enabled && button.IsDown();
  Color fill = is_down ? theme.selected_text_color : border;
  fill.a = is_down ? 0.7f : 0.12f;
  sys::Graphics::DrawRect(x, y, width, height, fill);

  if (label == nullptr) return;
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  DrawMenuText(x + (width - sys::Graphics::GetTextWidth(label)) / 2,
               y + (height - 14) / 2, label, border);
}

void Numpad::Draw() const {
  const Theme& theme = Theme::GetInstance();

  // The bar: the value of the entry is grey until the player types.
  const Button& bar = buttons_[kButtonInput];
  Color bar_color = theme.selected_text_color;
  bar_color.a = 0.2f;
  sys::Graphics::DrawRect(bar.GetX(), bar.GetY(), bar.GetWidth(),
                          bar.GetHeight(), bar_color);
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  DrawMenuText(bar.GetX() + 6, bar.GetY() + 3, input_,
               is_edited_ ? theme.selected_text_color
                          : theme.unselected_text_color);

  // The rectangles of the keys, then one text for each row of keys: one
  // text costs less than three (see main_application.cc).
  if (is_hex_) {
    for (u32 i = 0; i < SIZE(kHexKeys); i++) {
      DrawKey((ButtonId)kHexKeys[i], nullptr, true);
    }
    sys::Graphics::SetTextScale(0.6f, 0.6f);
    for (u32 row = 0; row < 4; row++) {
      c16 text[64] = {0};
      const Button& first = buttons_[kHexKeys[row * kHexColumns]];
      for (u32 column = 0; column < kHexColumns; column++) {
        const u8 key = kHexKeys[row * kHexColumns + column];
        const c16 label[2] = {GetKeyCharacter(key), 0};
        const Button& button = buttons_[key];
        const s32 center =
            button.GetX() - first.GetX() + button.GetWidth() / 2;
        sys::Graphics::AppendSpaces(
            text, SIZE(text), center - sys::Graphics::GetTextWidth(label) / 2);
        sys::Graphics::AppendText(text, SIZE(text), label);
      }
      DrawMenuText(first.GetX(), first.GetY() + (first.GetHeight() - 14) / 2,
                   text, theme.unselected_text_color);
    }
  } else {
    DrawDecimalKeys();
  }
  DrawKey(kButtonDelete, u"DEL", true);
  DrawKey(kButtonClear, u"CLR", true);
  DrawKey(kButtonOk, u"OK  (X)", true);
}

void Numpad::DrawDecimalKeys() const {
  const Theme& theme = Theme::GetInstance();
  for (u32 i = 0; i <= 9; i++) {
    DrawKey((ButtonId)(kButton0 + i), nullptr, true);
  }
  DrawKey(kButtonMinus, nullptr, allow_minus_);
  DrawKey(kButtonDot, nullptr, allow_dot_);
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  for (u32 row = 0; row < 4; row++) {
    c16 text[64] = {0};
    const Button& first = buttons_[kGridKeys[row * 3]];
    for (u32 column = 0; column < 3; column++) {
      const u8 key = kGridKeys[row * 3 + column];
      c16 label[2] = {0, 0};
      if (key <= 9) {
        label[0] = u'0' + key;
      } else if (key == kButtonMinus && allow_minus_) {
        label[0] = u'-';
      } else if (key == kButtonDot && allow_dot_) {
        label[0] = u'.';
      } else {
        continue; // A disabled key has no label.
      }
      const Button& button = buttons_[key];
      const s32 center = button.GetX() - first.GetX() + button.GetWidth() / 2;
      sys::Graphics::AppendSpaces(
          text, SIZE(text), center - sys::Graphics::GetTextWidth(label) / 2);
      sys::Graphics::AppendText(text, SIZE(text), label);
    }
    DrawMenuText(first.GetX(), first.GetY() + (first.GetHeight() - 14) / 2,
                 text, theme.unselected_text_color);
  }
}

void Numpad::Update() {
  for (u32 i = 0; i < kButtonMax; i++) {
    // The keys of the other mode are under the keys of this mode.
    const bool is_hex_key = i >= kButtonHexA;
    const bool is_shown = is_hex_ ? (i <= 9 || is_hex_key ||
                                     i >= kButtonInput)
                                  : !is_hex_key;
    if (is_shown) buttons_[i].Update();
  }

  for (u32 i = 0; i <= 9; i++) {
    if (buttons_[kButton0 + i].IsReleased()) {
      Theme::GetInstance().Play(kKeySound);
      AddChar(u'0' + i);
    }
  }
  for (u32 i = 0; is_hex_ && i < 6; i++) {
    if (buttons_[kButtonHexA + i].IsReleased()) {
      Theme::GetInstance().Play(kKeySound);
      AddChar(u'A' + i);
    }
  }

  if (is_hex_) {
    // No sign and no decimal point in the hexadecimal mode.
  } else if (allow_dot_ && buttons_[kButtonDot].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    bool has_dot = false;
    for (u32 i = 0; is_edited_ && i < cursor_; i++) {
      if (input_[i] == u'.') has_dot = true;
    }
    if (!has_dot) {
      if (!is_edited_ || cursor_ == 0 ||
          (cursor_ == 1 && input_[0] == u'-')) {
        AddChar(u'0');
      }
      AddChar(u'.');
    }
  }

  if (!is_hex_ && allow_minus_ && buttons_[kButtonMinus].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    // The minus key changes the sign of the number, at any time.
    is_edited_ = true;
    if (input_[0] == u'-') {
      memmove(input_, input_ + 1, cursor_ * sizeof(c16));
      cursor_--;
    } else if (cursor_ < kMaxLength) {
      memmove(input_ + 1, input_, (cursor_ + 1) * sizeof(c16));
      input_[0] = u'-';
      cursor_++;
    }
  }

  if (buttons_[kButtonDelete].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    is_edited_ = true;
    RemoveLastChar();
  }

  if (buttons_[kButtonClear].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    is_edited_ = true;
    cursor_ = 0;
    memset(input_, 0, sizeof(input_));
    if (is_hex_) {
      input_[cursor_++] = u'0';
      input_[cursor_++] = u'x';
    }
  }
}

bool Numpad::IsButtonOkReleased() const {
  return buttons_[kButtonOk].IsReleased();
}

void Numpad::SetInput(const c16* text, bool allow_minus, bool allow_dot) {
  memset(input_, 0, sizeof(input_));
  cursor_ = 0;
  while (text[cursor_] != 0 && cursor_ < kMaxLength) {
    input_[cursor_] = text[cursor_];
    cursor_++;
  }
  is_edited_ = false;
  allow_minus_ = allow_minus;
  allow_dot_ = allow_dot;
  for (u32 i = 0; i < kButtonMax; i++) buttons_[i].Reset();
}

void Numpad::AddChar(c16 character) {
  // The first key replaces the value of the entry.
  if (!is_edited_) {
    cursor_ = 0;
    memset(input_, 0, sizeof(input_));
    is_edited_ = true;
    if (is_hex_) {
      input_[cursor_++] = u'0';
      input_[cursor_++] = u'x';
    }
  }
  if (cursor_ >= kMaxLength) return;
  input_[cursor_] = character;
  cursor_++;
  input_[cursor_] = 0;
}

void Numpad::RemoveLastChar() {
  // The hexadecimal mode keeps "0x".
  if (cursor_ == 0 || (is_hex_ && cursor_ <= 2)) return;
  cursor_--;
  input_[cursor_] = 0;
}
} // namespace ui
