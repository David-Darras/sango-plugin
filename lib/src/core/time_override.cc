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
 * @file time_override.cc
 * @brief Sets the time of the game.
 *
 * The declarations are in core/patch/time_override.h.
 */

#include "core/patch/time_override.h"

#include "core/hook.h"
#include "system/address.h"

namespace core {

namespace {
core::Hook<void(s64*)> get_system_date_time_hook;
} // namespace

void TimeOverride::Initialize() {
  get_system_date_time_hook.Install(sys::address::kGetSystemDateTime,
                                    GetSystemDateTimeHook);
}

void TimeOverride::GetSystemDateTimeHook(s64* date_time) {
  get_system_date_time_hook(date_time);

  auto& feat = GetInstance();
  if (!feat.is_enabled) return;

  const s64 day_start = *date_time - *date_time % kMillisecondsPerDay;
  *date_time = day_start + feat.hour * kMillisecondsPerHour +
               feat.minute * kMillisecondsPerMinute;
}

} // namespace core
