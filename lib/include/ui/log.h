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
 * @file log.h
 * @brief The log of the plugin.
 */

#pragma once

#include <3ds.h>
#include <cstdarg>
#include <cstring>

#include "common.h"
#include "system/address.h"

namespace ui {
/**
 * @brief The log of the plugin: the last 13 messages. The page Log of the
 * menu shows them.
 *
 * @code
 * ui::Log::Print(u"Species %u", static_cast<u32>(species));
 * @endcode
 */
class Log {
  MAKE_SINGLETON(Log)
public:
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

  /// Returns the number of lines of the log (kMaxEntries).
  static constexpr u32 GetLineCount() { return kMaxEntries; }

  /// Returns a line of the log, the oldest first. An empty line has no text.
  static const c16* GetLine(u32 index) {
    return GetInstance().log_entries_[index < kMaxEntries ? index : 0];
  }

  /// Removes all the messages.
  static void Clear() {
    std::memset(GetInstance().log_entries_, 0,
                sizeof(GetInstance().log_entries_));
  }

  /// Adds a formatted message to the log of the plugin (64 characters at most).
  static void Print(const c16* message, ...) {
    if (!message)
      return;

    Log& log = GetInstance();

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
