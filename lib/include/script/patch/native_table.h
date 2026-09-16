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

#include <cstring>

#include "common.h"
#include "script/native/amx.h"

namespace script {

typedef PawnNativeFunction NativeFunction;

class NativeTable {
public:
  static NativeFunction Find(const c8* name) {
    static const uptr kTables[] = {
        core::address::kScriptNativesField,
        core::address::kScriptNativesState,
        core::address::kScriptNativesInteractive,
        core::address::kScriptNativesPokemonCenter,
        core::address::kScriptNativesMapEffects,
        core::address::kScriptNativesBattleFacility,
        core::address::kScriptNativesFieldServices,
        core::address::kScriptNativesNpcAi,
        core::address::kScriptNativesProgram,
    };
    for (uptr kTable : kTables) {
      NativeFunction function = FindIn((const PawnNativeBinding*)kTable,
                                       name);
      if (function != nullptr) return function;
    }
    return nullptr;
  }

  template <typename... Args>
  STATIC_INLINE s32 Call(NativeFunction function, AmxRuntime* amx,
                         Args... args) {
    PawnCell params[sizeof...(Args) + 1] = {
        (PawnCell)(sizeof...(Args) * sizeof(PawnCell)),
        (PawnCell)(s32)args...
    };
    return (s32)function(amx, params);
  }

private:
  static NativeFunction FindIn(const PawnNativeBinding* table,
                               const c8* name) {
    for (u32 i = 0; i < kMaxEntries; i++) {
      if (table[i].function_name == nullptr) break;
      if (std::strcmp(table[i].function_name, name) == 0) {
        return table[i].function;
      }
    }
    return nullptr;
  }

  static constexpr u32 kMaxEntries = 512;
};

} // namespace script
