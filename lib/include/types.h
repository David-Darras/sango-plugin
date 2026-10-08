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
 * @file types.h
 * @brief The base integer types (u8, u16, u32, s32...) for the whole project.
 *
 * The headers of libctrpf include a file with the name "types.h". The libctrpf
 * package does not always supply this file. This file supplies it.
 *
 * This file uses the types of libctru. Then it adds the macros that the
 * libctrpf headers need (for example NORETURN). You do not need a copy of
 * the CTRPluginFramework source code to build the plugin.
 */

#pragma once

#include <3ds/types.h>

#ifndef ALIGN
/// Aligns a type on m bytes.
#define ALIGN(m) __attribute__((aligned(m)))
#endif

#ifndef PACKED
/// Removes the padding bytes of a structure.
#define PACKED __attribute__((packed))
#endif

#ifndef USED
/// Keeps a symbol in the binary, also when no code uses it.
#define USED __attribute__((used))
#endif

#ifndef UNUSED
/// Stops the "unused" warning for a symbol.
#define UNUSED __attribute__((unused))
#endif

#ifndef DEPRECATED
/// Marks a function as deprecated.
#define DEPRECATED __attribute__((deprecated))
#endif

#ifndef NAKED
/// Removes the prologue and the epilogue of a function.
#define NAKED __attribute__((naked))
#endif

#ifndef NORETURN
/// Tells the compiler that a function never returns.
#define NORETURN __attribute__((noreturn))
#endif
