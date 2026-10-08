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
 * @file event_table.h
 * @brief The event flags and the script variables.
 */

#pragma once
#include "common.h"
#include "core/constant/event_flag.h"
#include "savedata/native/savedata.h"

namespace savedata {
/// The event flags and the script variables of the save data.
struct EventTable {
  SINGLETON(EventTable)
  void* vtable;
  u16 data[376];
  u8 flag[416];
  u16 _0;

  STATIC_INLINE EventTable& GetInstance() {
    return SaveData::GetInstance().GetEventTable();
  }

  /// Returns true when the flag is set.
  INLINE bool Check(EventFlag flag) {
    return ((bool(*)(EventTable*, EventFlag))core::address::kEventTableCheckFlag)(
        this, flag);
  }

  /// Clears the flag.
  INLINE void Reset(EventFlag flag) {
    ((void(*)(EventTable*, EventFlag))core::address::kEventTableResetFlag)(
        this, flag);
  }

  /// Sets the flag.
  INLINE void Set(EventFlag flag) {
    ((void(*)(EventTable*, EventFlag))core::address::kEventTableSetFlag)(
        this, flag);
  }
};
} // namespace savedata