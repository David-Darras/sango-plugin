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
#include "game/constant/event.h"
#include "game/savedata/savedata.h"

namespace savedata {
struct EventTable {
  SINGLETON(EventTable)
  void* vtable;
  u16 data[376];
  u8 flag[416];
  u16 _0;

  STATIC_INLINE EventTable& GetInstance() {
    return SaveData::GetInstance().GetEventTable();
  }

  INLINE bool Check(EventID flag_id) {
    return ((bool(*)(EventTable*, EventID))ADDRESS_EVENT_TABLE_CHECK_FLAG)(
        this, flag_id);
  }

  INLINE void Reset(EventID flag_id) {
    ((void(*)(EventTable*, EventID))ADDRESS_EVENT_TABLE_RESET_FLAG)(
        this, flag_id);
  }

  INLINE void Set(EventID flag_id) {
    ((void(*)(EventTable*, EventID))ADDRESS_EVENT_TABLE_SET_FLAG)(
        this, flag_id);
  }
};
} // namespace savedata