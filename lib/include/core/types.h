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
 * @file core/types.h
 * @brief The base types and macros of the project.
 *
 * | Type | Meaning |
 * |---|---|
 * | `u8`, `u16`, `u32`, `u64` | Unsigned integers of 8, 16, 32, 64 bits. |
 * | `s8`, `s16`, `s32`, `s64` | Signed integers of 8, 16, 32, 64 bits. |
 * | `f32`, `f64` | Decimal numbers of 32 and 64 bits. |
 * | `c8` | One character of an ASCII text. |
 * | `c16` | One character of a UTF-16 text (the text format of the game). |
 * | `uptr` | An address. |
 */

#pragma once

#include <types.h>
#include <cstdint>
#include <cstddef>

#define TYPEDEF_FLOAT(n, t) \
typedef t f##n;             \
typedef volatile t vf##n;

TYPEDEF_FLOAT(32, float)
TYPEDEF_FLOAT(64, double)

#undef TYPEDEF_FLOAT

/// One character of a UTF-16 text. Write a UTF-16 text as u"text".
typedef char16_t c16;
/// One character of an ASCII text.
typedef char c8;

/// An address in memory.
typedef uintptr_t uptr;

/// Asks the compiler to always copy the function into the caller.
#define INLINE inline __attribute__((always_inline))
/// Same as INLINE, for a static function.
#define STATIC_INLINE static inline __attribute__((always_inline))
/// Marks a symbol as weak: a different definition can replace it.
#define WEAK __attribute__((weak))

/// Returns the number of elements of a C array.
#define SIZE(x) ((sizeof(x)) / (sizeof((x)[0])))
