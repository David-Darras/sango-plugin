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

#ifndef SANGO_PLUGIN_UTILS_H
#define SANGO_PLUGIN_UTILS_H

#include <cstdarg>

#include "core.h"

/**
 * @class Utils
 * @brief Static utility class for string manipulation and binary introspection.
 */
class Utils {
 public:
  /**
   * @brief Formats a UTF-16 string using a variable argument list.
   * Wraps the internal vswprintf function.
   * @param pOut Target buffer to receive the formatted string.
   * @param pIn  Format string (Unicode).
   */
  static void Format(c16* pOut, const c16* pIn, ...) {
    va_list args;
    va_start(args, pIn);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        pOut, BUFFER_SIZE, pIn, args);
    va_end(args);
  }

  /**
   * @brief Calculates the length of a UTF-16 string (number of characters).
   * @param pIn The source Unicode string.
   * @return Number of characters before the null terminator.
   */
  static u32 GetLength(const c16* pIn) {
    return ((u32 (*)(const c16*))ADDRESS_STD_WCSLEN)(pIn);
  }

  /**
   * @brief Calculates the memory size of a UTF-16 string in bytes.
   * @param pIn The source Unicode string.
   * @return Size in bytes.
   */
  static u32 GetSize(const c16* pIn) { return (GetLength(pIn)) * sizeof(*pIn); }

  /**
   * @brief Extracts the class name from a C++ object's VTable.
   * Works by navigating through the RTTI (Type Info) structures.
   * @param vtable Pointer to the object's virtual table.
   * @return The class name as a C-string, or "???" if not found.
   */
  static const c8* GetClassNameFromVTable(void* vtable) {
    u32 addr = (uptr)vtable;
    // Check RTTI offset to ensure valid structure
    if (READ(u32, addr - 8) != 0) {
      return "???";
    }
    addr = READ(u32, addr - 4);  // Access TypeInfo pointer
    return (
        const c8*)(READ(u32, addr + 4));  // Access name string within TypeInfo
  }

  /**
   * @brief Extracts the parent class name from a C++ object's VTable.
   * Useful for debugging inheritance chains.
   * @param vtable Pointer to the object's virtual table.
   * @return The parent class name as a C-string, or "???" if not found.
   */
  static const c8* GetParentClassNameFromVTable(void* vtable) {
    u32 addr = (uptr)vtable;
    if (READ(u32, addr - 8) != 0) {
      return "???";
    }
    addr = READ(u32, addr - 4);
    return (const c8*)(READ(u32, addr + 8));  // Access base class name offset
  }
};

#endif  // SANGO_PLUGIN_UTILS_H
