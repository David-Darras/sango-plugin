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
 * @file bitmask.h
 * @brief Macros for bit fields and for enum classes of flags.
 */

#pragma once

#include <type_traits>
#include "core/types.h"

/// Writes the value `v` in the `n` bits of `b` that start at bit `p`.
#define SET_BITS(b, p, n, v) \
  ((b) = ((b) & ~(((1u << (n)) - 1) << (p))) | ((v) << (p)))
/// Reads the `n` bits of `b` that start at bit `p`.
#define GET_BITS(b, p, n) (((b) >> (p)) & ((1u << (n)) - 1))

/**
 * @brief Adds the bit operators (|, &, ^, ~) and HasFlag() to an enum class.
 *
 * Use it after an enum class of flags:
 * @code
 * enum class MyFlags : u32 { kNone = 0, kFirst = 1, kSecond = 2 };
 * ENABLE_BITMASK_OPERATORS(MyFlags)
 *
 * if (HasFlag(flags, MyFlags::kSecond)) { ... }
 * @endcode
 */
#define ENABLE_BITMASK_OPERATORS(EnumType) \
  inline EnumType operator|(EnumType a, EnumType b) { \
    typedef typename std::underlying_type<EnumType>::type Underlying; \
    return static_cast<EnumType>(static_cast<Underlying>(a) | static_cast<Underlying>(b)); \
  } \
  inline EnumType operator&(EnumType a, EnumType b) { \
    typedef typename std::underlying_type<EnumType>::type Underlying; \
    return static_cast<EnumType>(static_cast<Underlying>(a) & static_cast<Underlying>(b)); \
  } \
  inline EnumType operator^(EnumType a, EnumType b) { \
    typedef typename std::underlying_type<EnumType>::type Underlying; \
    return static_cast<EnumType>(static_cast<Underlying>(a) ^ static_cast<Underlying>(b)); \
  } \
  inline EnumType operator~(EnumType a) { \
    typedef typename std::underlying_type<EnumType>::type Underlying; \
    return static_cast<EnumType>(~static_cast<Underlying>(a)); \
  } \
  inline EnumType& operator|=(EnumType& a, EnumType b) { \
    return a = (a | b); \
  } \
  inline EnumType& operator&=(EnumType& a, EnumType b) { \
    return a = (a & b); \
  } \
  inline EnumType& operator^=(EnumType& a, EnumType b) { \
    return a = (a ^ b); \
  } \
  inline bool HasFlag(EnumType value, EnumType flag) { \
    typedef typename std::underlying_type<EnumType>::type Underlying; \
    return (static_cast<Underlying>(value) & static_cast<Underlying>(flag)) != 0; \
  }
