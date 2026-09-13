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

#include <types.h>
#include <cstdint>
#include <cstddef>

#define TYPEDEF_FLOAT(n, t) \
typedef t f##n;             \
typedef volatile t vf##n;

TYPEDEF_FLOAT(32, float)
TYPEDEF_FLOAT(64, double)

#undef TYPEDEF_FLOAT

typedef char16_t c16;
typedef char c8;

typedef uintptr_t uptr;

#define INLINE inline __attribute__((always_inline))
#define STATIC_INLINE static inline __attribute__((always_inline))
#define WEAK __attribute__((weak))

#define SIZE(x) ((sizeof(x)) / (sizeof((x)[0])))
