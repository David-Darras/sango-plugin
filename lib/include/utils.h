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

#include <cstdarg>
#include <cstring>

#include "common.h"
#include <cxxabi.h>

#include "game/constant/item.h"


class Utils {
public:
  static void Format(c16* pOut, const c16* pIn, ...) {
    va_list args;
    va_start(args, pIn);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        pOut, BUFFER_SIZE, pIn, args);
    va_end(args);
  }

  static String* FormatString(const c16* pIn, ...) {
    va_list args;
    va_start(args, pIn);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        String::GetTmpBuf(), 128, pIn, args);
    va_end(args);
    return String::GetTmpStr();
  }

  STATIC_INLINE u32 GetLength(const c16* pIn) {
    return ((u32 (*)(const c16*))ADDRESS_STD_WCSLEN)(pIn);
  }

  STATIC_INLINE u32 GetSize(const c16* pIn) {
    return (GetLength(pIn)) * sizeof(*pIn);
  }

  static const c8* GetClassNameFromVTable(void* vtable) {
    u32 addr = (uptr)vtable;
    if (addr < ADDRESS_PROCESS_MEMORY_START || addr >
        ADDRESS_PROCESS_MEMORY_END)
      return "";
    addr = READ32(addr - 4);
    if (addr == 0) return "";
    addr = READ32(addr + 4);
    if (addr == 0) return "";
    return (const char*)addr;
  }

  // Example : overworld_models_
  STATIC_INLINE u32 GetArrayElementSize(uptr addr) { return READ32(addr - 8); }

  STATIC_INLINE u32 GetArrayCapacity(uptr addr) { return READ32(addr - 4); }

  STATIC_INLINE u32 GetRandomValue(u32 max = 0xFFFFFFFF) {
    return ((u32(*)(u32))ADDRESS_GET_RANDOM_VALUE)(max);
  }

  STATIC_INLINE void GetElapsedTime(s64* time) {
    return ((void(*)(s64*))ADDRESS_GET_ELAPSED_TIME)(time);
  }

  STATIC_INLINE s32 ConvertTimeToSeconds(s64* time) {
    return ((s32(*)(s64*))ADDRESS_CONVERT_TIME_TO_SECONDS)(time);
  }

  static const char* Unmangle(const char* mangled_name) {
    static char buffer[BUFFER_SIZE];

    if (!mangled_name) {
      return "";
    }

    int status = -1;
    char* demangled =
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);

    if (status == 0 && demangled != nullptr) {
      strncpy(buffer, demangled, BUFFER_SIZE - 1);
      buffer[BUFFER_SIZE - 1] = '\0';
      std::free(demangled);
    } else {
      strncpy(buffer, mangled_name, BUFFER_SIZE - 1);
      buffer[BUFFER_SIZE - 1] = '\0';
    }

    return buffer;
  }

  static ItemId GetRandomItemId() {
    const u16 max = static_cast<u16>(ItemId::kCount);
    const u16 min = 1;
    const u16 rand = min + GetRandomValue(max);
    return static_cast<ItemId>(rand);
  }

  static Species GetRandomSpecies() {
    const u16 max = static_cast<u16>(Species::kCount);
    const u16 min = 1;
    const u16 rand = min + GetRandomValue(max);
    return static_cast<Species>(rand);
  }
};