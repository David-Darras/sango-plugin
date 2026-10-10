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
 * @file keyboard.cc
 * @brief The keyboard of the menu on the bottom screen.
 *
 * The declarations are in ui/widget/keyboard.h.
 */

#include "ui/widget/keyboard.h"

#include <cstring>

#include "core/utils.h"
#include "system/native/font_manager.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "ui/theme.h"

namespace ui {
namespace {
// 30 large keys: each DrawText() fills the command list of the GPU (see
// main_application.cc), and large keys are easier to touch.
constexpr s32 kKeyWidth = 30;
constexpr s32 kKeyHeight = 26;
constexpr s32 kBarHeight = 20;
constexpr u16 kKeySound = 4;
constexpr u32 kLastChar = 0xFFFF;

// The groups of characters. << and >> jump from one group to the next.
struct Group {
  u16 first;
  const c8* name;
};

const Group kGroups[] = {
    {0x0020, "Latin"},       {0x00A0, "Latin-1"},
    {0x0100, "Latin Ext."},  {0x0370, "Greek"},
    {0x0400, "Cyrillic"},    {0x2000, "Punctuation"},
    {0x2190, "Arrows"},      {0x2460, "Numbers"},
    {0x2500, "Shapes"},      {0x3000, "CJK Symbols"},
    {0x3040, "Hiragana"},    {0x30A0, "Katakana"},
    {0x4E00, "Kanji"},       {0xAC00, "Hangul"},
    {0xE000, "Game Icons"},  {0xFF00, "Full Width"},
};

// Returns the first character at or after `character` that the font can
// draw. The search stops at `end`: the result is then `end`.
u32 FindPrintable(u32 character, u32 end = kLastChar + 1) {
  while (character < end &&
         !sys::FontManager::IsPrintable((u16)character)) {
    character++;
  }
  return character;
}
} // namespace

Keyboard::Keyboard() : first_char_(0x20), cursor_(0), capacity_(kBufferSize) {
  memset(input_, 0, sizeof(input_));
  memset(chars_, 0, sizeof(chars_));
  Initialize(10, 10);
}

void Keyboard::Initialize(s32 x, s32 y) {
  buttons_[kButtonInput].Initialize(x, y, 300, kBarHeight);

  const s32 grid_y = y + kBarHeight + 4;
  for (u32 row = 0; row < kRowNum; row++) {
    for (u32 col = 0; col < kColNum; col++) {
      const u32 index = row * kColNum + col;
      buttons_[kButtonGridStart + index].Initialize(
          x + col * kKeyWidth, grid_y + row * kKeyHeight, kKeyWidth,
          kKeyHeight);
    }
  }

  // The page buttons and the edit buttons, under the characters.
  const s32 row_y = grid_y + kRowNum * kKeyHeight + 4;
  const s32 height = 22;
  const s32 nav_width = 32;
  const s32 edit_width = 50;
  const s32 gap = 4;
  s32 left = x;
  buttons_[kButtonPrevGroup].Initialize(left, row_y, nav_width, height);
  left += nav_width + gap;
  buttons_[kButtonPrev].Initialize(left, row_y, nav_width, height);
  left += nav_width + gap;
  buttons_[kButtonDelete].Initialize(left, row_y, edit_width, height);
  left += edit_width + gap;
  buttons_[kButtonClear].Initialize(left, row_y, edit_width, height);
  left += edit_width + gap;
  buttons_[kButtonOk].Initialize(left, row_y, edit_width, height);
  left += edit_width + gap;
  buttons_[kButtonNext].Initialize(left, row_y, nav_width, height);
  left += nav_width + gap;
  buttons_[kButtonNextGroup].Initialize(left, row_y, nav_width, height);
}

void Keyboard::DrawKey(u32 id, const c16* label, u32 offset_x) const {
  const Theme& theme = Theme::GetInstance();
  const Button& button = buttons_[id];
  // Only the pressed key has a rectangle: 60 keys with a border each would
  // fill the command list of the GPU (see main_application.cc).
  if (button.IsDown()) {
    Color fill = theme.selected_text_color;
    fill.a = 0.7f;
    sys::Graphics::DrawRect(button.GetX(), button.GetY(), button.GetWidth(),
                            button.GetHeight(), fill);
  }
  if (label == nullptr) return;
  // The label is in the center of the key. `offset_x` is the margin when
  // the label is wider than the key.
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  s32 label_x = button.GetX() +
                (button.GetWidth() - sys::Graphics::GetTextWidth(label)) / 2;
  if (label_x < (s32)button.GetX() + 1) label_x = button.GetX() + offset_x;
  DrawMenuText(label_x, button.GetY() + (button.GetHeight() - 14) / 2, label,
               theme.unselected_text_color);
}

void Keyboard::Draw() const {
  const Theme& theme = Theme::GetInstance();

  // The bar: the text, then the group and the code of the page.
  const Button& bar = buttons_[kButtonInput];
  Color bar_color = theme.selected_text_color;
  bar_color.a = 0.2f;
  sys::Graphics::DrawRect(bar.GetX(), bar.GetY(), bar.GetWidth(),
                          bar.GetHeight(), bar_color);
  // One rectangle behind all the characters.
  Color grid = theme.unselected_text_color;
  grid.a = 0.08f;
  const Button& first = buttons_[kButtonGridStart];
  sys::Graphics::DrawRect(first.GetX(), first.GetY(), kColNum * kKeyWidth,
                          kRowNum * kKeyHeight, grid);
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  DrawMenuText(bar.GetX() + 6, bar.GetY() + 3, input_,
               theme.selected_text_color);
  c16 info[48];
  core::Utils::Format(info, u"%s U+%04X  %d/%d", kGroups[FindGroup(first_char_)].name,
                      first_char_, cursor_, capacity_ - 1);
  sys::Graphics::SetTextScale(0.4f, 0.4f);
  DrawMenuText(bar.GetX() + 180, bar.GetY() + 5, info,
               theme.unselected_text_color);

  // The pressed key, then one text for each row of characters: one text
  // costs less than ten (see main_application.cc).
  for (u32 i = 0; i < kPageSize; i++) {
    if (chars_[i] != 0) DrawKey(kButtonGridStart + i, nullptr, 0);
  }
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  for (u32 row = 0; row < kRowNum; row++) {
    c16 text[96] = {0};
    for (u32 column = 0; column < kColNum; column++) {
      const c16 character = chars_[row * kColNum + column];
      if (character == 0) continue;
      const c16 label[2] = {character, 0};
      const s32 center = column * kKeyWidth + kKeyWidth / 2;
      sys::Graphics::AppendSpaces(
          text, SIZE(text), center - sys::Graphics::GetTextWidth(label) / 2);
      sys::Graphics::AppendText(text, SIZE(text), label);
    }
    DrawMenuText(first.GetX(),
                 first.GetY() + row * kKeyHeight + (kKeyHeight - 14) / 2, text,
                 theme.unselected_text_color);
  }

  DrawKey(kButtonPrevGroup, u"<<", 7);
  DrawKey(kButtonPrev, u"<", 12);
  DrawKey(kButtonDelete, u"DEL", 12);
  DrawKey(kButtonClear, u"CLR", 12);
  DrawKey(kButtonOk, u"OK", 16);
  DrawKey(kButtonNext, u">", 12);
  DrawKey(kButtonNextGroup, u">>", 7);
}

void Keyboard::Update() {
  if (chars_[0] == 0) ShowPage(first_char_);

  for (u32 i = 0; i < kButtonMax; i++) {
    buttons_[i].Update();
  }

  for (u32 i = 0; i < kPageSize; i++) {
    if (chars_[i] != 0 && buttons_[kButtonGridStart + i].IsReleased()) {
      Theme::GetInstance().Play(kKeySound);
      AddChar(chars_[i]);
    }
  }

  if (buttons_[kButtonPrev].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    ShowPreviousPage();
  }
  if (buttons_[kButtonNext].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    const c16 last = chars_[kPageSize - 1];
    // After the last page, go back to the first page.
    ShowPage(last == 0 ? 0x20 : last + 1);
  }
  if (buttons_[kButtonPrevGroup].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    JumpGroup(-1);
  }
  if (buttons_[kButtonNextGroup].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    JumpGroup(1);
  }

  if (buttons_[kButtonDelete].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    RemoveLastChar();
  }
  if (buttons_[kButtonClear].IsReleased()) {
    Theme::GetInstance().Play(kKeySound);
    cursor_ = 0;
    memset(input_, 0, sizeof(input_));
  }
}

bool Keyboard::IsButtonOkReleased() const {
  return buttons_[kButtonOk].IsReleased();
}

void Keyboard::SetInput(const c16* text, u32 capacity) {
  capacity_ = (capacity == 0 || capacity > kBufferSize) ? kBufferSize
                                                        : capacity;
  memset(input_, 0, sizeof(input_));
  cursor_ = 0;
  while (cursor_ + 1 < capacity_ && text[cursor_] != 0) {
    input_[cursor_] = text[cursor_];
    cursor_++;
  }
  for (u32 i = 0; i < kButtonMax; i++) buttons_[i].Reset();
}

const c16* Keyboard::GetInput() const { return input_; }

void Keyboard::ShowPage(u32 first) {
  u32 character = FindPrintable(first);
  if (character > kLastChar) character = FindPrintable(0x20);
  first_char_ = (u16)character;
  for (u32 i = 0; i < kPageSize; i++) {
    if (character > kLastChar) {
      chars_[i] = 0;
      continue;
    }
    chars_[i] = (c16)character;
    character = FindPrintable(character + 1);
  }
}

void Keyboard::ShowPreviousPage() {
  // Count kPageSize printable characters before the first character.
  u32 count = 0;
  u32 first = first_char_;
  for (u32 character = first_char_; character > 0x20 && count < kPageSize;) {
    character--;
    if (sys::FontManager::IsPrintable((u16)character)) {
      first = character;
      count++;
    }
  }
  if (count == 0) {
    // The first page: go to the last group.
    JumpGroup(-1);
    return;
  }
  ShowPage(first);
}

u32 Keyboard::FindGroup(u32 character) {
  u32 group = 0;
  for (u32 i = 0; i < SIZE(kGroups); i++) {
    if (kGroups[i].first <= character) group = i;
  }
  return group;
}

void Keyboard::JumpGroup(s32 direction) {
  const u32 count = SIZE(kGroups);
  u32 group = FindGroup(first_char_);
  if (direction < 0 &&
      FindPrintable(kGroups[group].first, first_char_) < first_char_) {
    // First go to the start of the current group.
    ShowPage(kGroups[group].first);
    return;
  }
  // Skip the groups that have no character in the font.
  for (u32 n = 0; n < count; n++) {
    group = (group + count + direction) % count;
    const u32 end = group + 1 < count ? kGroups[group + 1].first : kLastChar + 1;
    if (FindPrintable(kGroups[group].first, end) < end) break;
  }
  ShowPage(kGroups[group].first);
}

void Keyboard::AddChar(c16 character) {
  if (cursor_ + 1 >= capacity_) return;
  if (character == 0) return;
  input_[cursor_] = character;
  cursor_++;
  input_[cursor_] = 0;
}

void Keyboard::RemoveLastChar() {
  if (cursor_ == 0) return;
  cursor_--;
  input_[cursor_] = 0;
}
} // namespace ui
