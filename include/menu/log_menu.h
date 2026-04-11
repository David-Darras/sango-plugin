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

#ifndef SANGO_PLUGIN_MENU_LOG_MENU_H
#define SANGO_PLUGIN_MENU_LOG_MENU_H

#include "common.h"

namespace menu {

class LogMenu {
 public:
  static LogMenu& GetInstance() { return instance_; }

  void Toggle() { is_enabled_ ^= true; }
  bool IsEnabled() const { return is_enabled_; }

  void Draw();
  void Add(const c16* message, ...);

 private:
  LogMenu();

  static constexpr u32 kMaxEntries = 13;
  static constexpr u32 kMaxEntryLength = 64;
  static constexpr u32 kLineHeight = 18;
  static LogMenu instance_;

  bool is_enabled_;
  c16 log_entries_[kMaxEntries][kMaxEntryLength];
};

}  // namespace menu

#endif  // SANGO_PLUGIN_MENU_LOG_MENU_H