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

#include "menu/keyboard.h"

#include <cstring>

#include "system/graphics.h"
#include "system/sound.h"
#include "utils.h"

namespace menu {

Keyboard::Keyboard() : page_index_(0), cursor_(0) {
  memset(input_, 0, sizeof(input_));

  u32 x_start = 10;
  u32 y_start = 10;
  u32 key_width = 20;
  u32 key_height = 20;
  u32 bar_height = 18;

  buttons_[kButtonInput].Initialize(x_start, y_start, 300, bar_height);

  s32 y = y_start + bar_height + 5;
  for (int row = 0; row < kRowNum; row++) {
    for (int col = 0; col < kColNum; col++) {
      int index = row * kColNum + col;
      buttons_[kButtonGridStart + index].Initialize(x_start + (col * key_width),
                                                    y + (row * key_height),
                                                    key_width, key_height);
    }
  }

  // 3. Action & Navigation Buttons
  y += (kRowNum * key_height) + 5;
  s32 btn_height = 25;
  s32 nav_btn_width = 30;
  s32 act_btn_width = 48;
  s32 gap = 4;
  s32 x = x_start;

  buttons_[kButtonPrev10].Initialize(x, y, nav_btn_width, btn_height);
  x += nav_btn_width + gap;
  buttons_[kButtonPrev].Initialize(x, y, nav_btn_width, btn_height);
  x += nav_btn_width + gap;

  buttons_[kButtonDelete].Initialize(x, y, act_btn_width, btn_height);
  x += act_btn_width + gap;
  buttons_[kButtonCancel].Initialize(x, y, act_btn_width, btn_height);
  x += act_btn_width + gap;
  buttons_[kButtonOk].Initialize(x, y, act_btn_width, btn_height);
  x += act_btn_width + gap;

  buttons_[kButtonNext].Initialize(x, y, nav_btn_width, btn_height);
  x += nav_btn_width + gap;
  buttons_[kButtonNext10].Initialize(x, y, nav_btn_width, btn_height);
}

void Keyboard::Draw() const {
  c16 buffer[2] = {0, 0};

  // Draw input area
  buttons_[kButtonInput].Draw(input_, 2, 0);

  // Draw character grid
  c16 ch = page_index_ * kPageSize;
  for (c16 i = 0; i < kPageSize; i++) {
    buffer[0] = ch + i;
    buttons_[kButtonGridStart + i].Draw(buffer, 6, 4);
  }

  // Labels
  buttons_[kButtonPrev10].Draw(u"<<", 6, 4);
  buttons_[kButtonPrev].Draw(u"<", 10, 4);
  buttons_[kButtonDelete].Draw(u"DEL", 12, 4);
  buttons_[kButtonCancel].Draw(u"CLR", 12, 4);
  buttons_[kButtonOk].Draw(u"OK", 15, 4);
  buttons_[kButtonNext].Draw(u">", 10, 4);
  buttons_[kButtonNext10].Draw(u">>", 6, 4);

  // Draw the page num
  c16 page[BUFFER_SIZE];
  Utils::Format(page, u"[u%04X] %d/%d", page_index_ * kPageSize, page_index_,
                0x10000 / kPageSize);
  Graphics::DrawText(5, 240 - 20, page);
}

void Keyboard::Update() {
  for (u32 i = 0; i < kButtonMax; i++) {
    buttons_[i].Update();
  }

  u32 ch = page_index_ * kPageSize;
  for (u32 i = 0; i < kPageSize; i++) {
    if (buttons_[kButtonGridStart + i].IsReleased()) {
      Sound::PlaySoundEffect(4);
      AddChar(ch + i);
    }
  }

  // Navigation logic
  if (buttons_[kButtonPrev].IsReleased()) {
    Sound::PlaySoundEffect(4);
    if (page_index_ > 0) page_index_--;
  }
  if (buttons_[kButtonNext].IsReleased()) {
    Sound::PlaySoundEffect(4);
    page_index_++;
  }
  if (buttons_[kButtonPrev10].IsReleased()) {
    Sound::PlaySoundEffect(4);
    if (page_index_ >= 10)
      page_index_ -= 10;
    else
      page_index_ = 0;
  }
  if (buttons_[kButtonNext10].IsReleased()) {
    Sound::PlaySoundEffect(4);
    page_index_ += 10;
  }

  // Edit actions
  if (buttons_[kButtonDelete].IsReleased()) {
    Sound::PlaySoundEffect(4);
    RemoveLastChar();
  }
  if (buttons_[kButtonCancel].IsReleased()) {
    Sound::PlaySoundEffect(4);
    cursor_ = 0;
    memset(input_, 0, sizeof(input_));
  }
}

bool Keyboard::IsButtonOkReleased() const {
  return buttons_[kButtonOk].IsReleased();
}

const c16* Keyboard::GetInput() const { return input_; }

void Keyboard::AddChar(c16 character) {
  if (cursor_ >= kBufferSize - 1) return;
  if (character == 0) return;
  input_[cursor_] = character;
  cursor_++;
  input_[cursor_] = 0;
}

void Keyboard::RemoveLastChar() {
  if (cursor_ <= 0) return;
  cursor_--;
  input_[cursor_] = 0;
}

}  // namespace menu