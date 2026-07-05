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

#include "common.h"

class Utils {
public:
  static void Format(c16* pOut, const c16* pIn, ...) {
    va_list args;
    va_start(args, pIn);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        pOut, BUFFER_SIZE, pIn, args);
    va_end(args);
  }

  static u32 GetLength(const c16* pIn) {
    return ((u32 (*)(const c16*))ADDRESS_STD_WCSLEN)(pIn);
  }

  static u32 GetSize(const c16* pIn) { return (GetLength(pIn)) * sizeof(*pIn); }

  static const c8* GetClassNameFromVTable(void* vtable) {
    u32 addr = (uptr)vtable;
    if (addr < 0x00100000 || addr > 0x00900000)return "";
    addr = READ(u32, addr - 4);
    if (addr == 0) return "";
    addr = READ(u32, addr + 4);
    if (addr == 0) return "";
    return (const char*)addr;
  }

  // Example : overworld_models_
  static u32 GetArrayElementSize(uptr addr) { return READ(u32, addr - 8); }

  static u32 GetArrayCapacity(uptr addr) { return READ(u32, addr - 4); }

  static u32 GetRandomValue(u32 max = 0xFFFFFFFF) {
    return ((u32(*)(u32))ADDRESS_GET_RANDOM_VALUE)(max);
  }
};