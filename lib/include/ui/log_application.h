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
 * @file log_application.h
 * @brief The log window of the plugin.
 */

#pragma once

#include <3ds.h>
#include <cstdarg>
#include <cstring>

#include "common.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "ui/application.h"

namespace ui {
/**
 * @brief The log window: the last 13 messages. Press L + R to show it.
 *
 * @code
 * ui::LogApplication::Print(u"Species %u", static_cast<u32>(species));
 * @endcode
 */
class LogApplication : public Application {
  MAKE_SINGLETON(LogApplication)
public:
  void DrawTop(sys::Graphics& graphics) override {
    sys::Graphics::FillScreen(0, 0, 1, 0.6f);

    Color text_color{1, 1, 1, 1};

    for (u32 i = 0; i < kMaxEntries; i++) {
      if (log_entries_[i][0] == u'\0') continue;

      int x = 5;
      int y = 5 + i * kLineHeight;

      sys::Graphics::DrawText(x, y, log_entries_[i], text_color);
    }
  }

  void DrawBottom(sys::Graphics& graphics) override {
    sys::Graphics::FillScreen(0, 0, 1, 0.6f);

    Color text_color{1, 1, 1, 1};
    sys::Graphics::DrawText(10, 10, u"[DEBUG VIEW]", text_color);
  }

  void Update(sys::Controller& controller) override {
  }

  /// Adds a formatted message (64 characters at most).
  void Add(const c16* message, ...) {
    if (!message) return;

    c16 buffer[sys::address::kBufferSize];

    va_list args;
    va_start(args, message);
    ((void (*)(c16*, u32, const c16*, va_list))sys::address::kStdVswprintf)(
        buffer, sys::address::kBufferSize, message, args);
    va_end(args);

    for (u32 i = 0; i < kMaxEntries - 1; i++) {
      std::memcpy(log_entries_[i], log_entries_[i + 1],
                  sizeof(c16) * kMaxEntryLength);
    }

    std::memcpy(log_entries_[kMaxEntries - 1], buffer,
                sizeof(c16) * kMaxEntryLength);
  }

  /// Adds a formatted message to the log of the plugin (64 characters at most).
  static void Print(const c16* message, ...) {
    if (!message)
      return;

    LogApplication& log = GetInstance();

    c16 buffer[kMaxEntryLength];

    va_list args;
    va_start(args, message);

    ((void (*)(c16*, u32, const c16*, va_list))sys::address::kStdVswprintf)(
        buffer, kMaxEntryLength, message, args);

    va_end(args);

    for (u32 i = 0; i < kMaxEntries - 1; i++) {
      std::memcpy(log.log_entries_[i],
                  log.log_entries_[i + 1],
                  sizeof(c16) * kMaxEntryLength);
    }

    std::memcpy(log.log_entries_[kMaxEntries - 1],
                buffer,
                sizeof(c16) * kMaxEntryLength);

    // The debug output too: an emulator writes it in its log file.
    c8 text[kMaxEntryLength];
    u32 length = 0;
    for (; length + 1 < kMaxEntryLength && buffer[length] != 0; length++) {
      text[length] = buffer[length] < 0x80 ? (c8)buffer[length] : '?';
    }
    text[length] = '\0';
    svcOutputDebugString(text, length);
  }

private:
  static constexpr u32 kMaxEntries = 13;
  static constexpr u32 kMaxEntryLength = 64;
  static constexpr u32 kLineHeight = 18;

  c16 log_entries_[kMaxEntries][kMaxEntryLength] = {};
};
} // namespace ui
