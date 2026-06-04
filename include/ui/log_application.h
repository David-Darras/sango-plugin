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

#include <cstdarg>
#include <cstring>

#include "application.h"
#include "common.h"
#include "system/device.h"
#include "system/graphics.h"

namespace ui {
class LogApplication : public Application {
  MAKE_SINGLETON(LogApplication)
public:
  void DrawTop(Graphics& graphics) override {
    Graphics::FillScreen(0, 0, 1, 0.6f);

    Color text_color{1, 1, 1, 1};

    for (u32 i = 0; i < kMaxEntries; i++) {
      if (log_entries_[i][0] == u'\0') continue;

      int x = 5;
      int y = 5 + i * kLineHeight;

      Graphics::DrawText(x, y, log_entries_[i], text_color);
    }
  }

  void DrawBottom(Graphics& graphics) override {
    Graphics::FillScreen(0, 0, 1, 0.6f);

    Color text_color{1, 1, 1, 1};
    Graphics::DrawText(10, 10, u"[DEBUG VIEW]", text_color);
  }

  void Update(Controller& controller) override {
  }

  void Add(const c16* message, ...) {
    if (!message) return;

    c16 buffer[BUFFER_SIZE];

    va_list args;
    va_start(args, message);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        buffer, BUFFER_SIZE, message, args);
    va_end(args);

    for (u32 i = 0; i < kMaxEntries - 1; i++) {
      std::memcpy(log_entries_[i], log_entries_[i + 1],
                  sizeof(c16) * kMaxEntryLength);
    }

    std::memcpy(log_entries_[kMaxEntries - 1], buffer,
                sizeof(c16) * kMaxEntryLength);
  }

private:
  static constexpr u32 kMaxEntries = 13;
  static constexpr u32 kMaxEntryLength = 64;
  static constexpr u32 kLineHeight = 18;

  c16 log_entries_[kMaxEntries][kMaxEntryLength] = {};
};
} // namespace ui