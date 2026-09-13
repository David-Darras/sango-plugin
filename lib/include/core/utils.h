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

#include "pokemon/constant/item.h"

namespace core {

class Utils {
public:
  static void Format(c16* pOut, const c16* pIn, ...) {
    va_list args;
    va_start(args, pIn);
    ((void (*)(c16*, u32, const c16*, va_list))sys::address::kStdVswprintf)(
        pOut, sys::address::kBufferSize, pIn, args);
    va_end(args);
  }

  static String* FormatString(const c16* pIn, ...) {
    va_list args;
    va_start(args, pIn);
    ((void (*)(c16*, u32, const c16*, va_list))sys::address::kStdVswprintf)(
        String::GetTmpBuf(), 128, pIn, args);
    va_end(args);
    return String::GetTmpStr();
  }

  STATIC_INLINE u32 GetLength(const c16* pIn) {
    return ((u32 (*)(const c16*))sys::address::kStdWcslen)(pIn);
  }

  static const c8* GetClassNameFromVTable(void* vtable) {
    u32 addr = (uptr)vtable;
    if (addr < sys::address::kProcessMemoryStart || addr >
        sys::address::kProcessMemoryEnd)
      return "";
    addr = READ32(addr - 4);
    if (addr == 0) return "";
    addr = READ32(addr + 4);
    if (addr == 0) return "";
    return (const char*)addr;
  }

  STATIC_INLINE u32 GetArrayElementSize(uptr addr) { return READ32(addr - 8); }

  STATIC_INLINE u32 GetArrayCapacity(uptr addr) { return READ32(addr - 4); }

  STATIC_INLINE u32 GetRandomValue(u32 max = 0xFFFFFFFF) {
    return ((u32(*)(u32))sys::address::kGetRandomValue)(max);
  }

  /// A random enumerator in [1, T::kCount): skips the "none" entry at 0.
  template <typename T>
  static T GetRandomEnum() {
    constexpr u32 min = 1;
    const u32 max = static_cast<u32>(T::kCount);
    return static_cast<T>(min + GetRandomValue(max - min));
  }

  STATIC_INLINE void GetElapsedTime(s64* time) {
    return ((void(*)(s64*))sys::address::kGetElapsedTime)(time);
  }

  STATIC_INLINE s32 ConvertTimeToSeconds(s64* time) {
    return ((s32(*)(s64*))sys::address::kConvertTimeToSeconds)(time);
  }

  static const char* Unmangle(const char* mangled_name) {
    static char buffer[sys::address::kBufferSize];

    if (!mangled_name) {
      return "";
    }

    int status = -1;
    char* demangled =
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);

    if (status == 0 && demangled != nullptr) {
      strncpy(buffer, demangled, sys::address::kBufferSize - 1);
      buffer[sys::address::kBufferSize - 1] = '\0';
      std::free(demangled);
    } else {
      strncpy(buffer, mangled_name, sys::address::kBufferSize - 1);
      buffer[sys::address::kBufferSize - 1] = '\0';
    }

    return buffer;
  }
};

} // namespace core
