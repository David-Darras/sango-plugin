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

#include "core/types.h"
#include "script/constant/script_kind.h"

namespace script {

struct ScriptDescriptor {
  ScriptKind kind; // 0x00
  u8 _0[3];
  s32 script_id; // 0x04
  u32 message_archive_id; // 0x08
  u32 message_file_id; // 0x0C
  u32 native_set_bits; // 0x10
  u16 zone_id; // 0x14
  u16 amx_id; // 0x16
  const c8* amx_file_name; // 0x18
  u16 table_index; // 0x1C
  u16 _1;
};

static_assert(sizeof(ScriptDescriptor) == 0x20, "ScriptDescriptor layout");

} // namespace script
