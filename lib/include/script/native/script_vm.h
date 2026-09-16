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
#include "script/native/pawn_vm.h"
#include "script/native/script_run.h"

namespace script {
class Engine;

typedef bool (*WaitFunction)(Engine* engine, u32 label, void* work);

struct ScriptVm : PawnVm {
  ScriptRun* run; // 0x90
  bool is_loaded; // 0x94
  u8 _0[3];
  u32 native_set_bits; // 0x98
  Engine* engine; // 0x9C
  WaitFunction wait_function; // 0xA0
  u32 wait_label; // 0xA4
  void* wait_work; // 0xA8

  INLINE ScriptDescriptor* GetDescriptor() const {
    return run != nullptr ? &run->descriptor : nullptr;
  }

  STATIC_INLINE ScriptVm* FromAmx(AmxRuntime* amx) {
    return (ScriptVm*)amx->user_data_ptrs[0];
  }
};

static_assert(sizeof(ScriptVm) == 0xAC, "ScriptVm layout");

} // namespace script
