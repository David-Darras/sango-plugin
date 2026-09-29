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

namespace core {

struct TimeOverride {
  MAKE_SINGLETON(TimeOverride)

  bool is_enabled = false;
  u8 hour = 12;
  u8 minute = 0;

  static void Initialize();

private:
  static constexpr s64 kMillisecondsPerMinute = 60 * 1000;
  static constexpr s64 kMillisecondsPerHour = 60 * kMillisecondsPerMinute;
  static constexpr s64 kMillisecondsPerDay = 24 * kMillisecondsPerHour;

  static void GetSystemDateTimeHook(s64* date_time);
};

} // namespace core
