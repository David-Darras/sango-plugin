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
 * @file formula.h
 * @brief A small calculation that Sango Studio writes: a formula of integers.
 *
 * The page "Formula" of Sango Studio turns the text of a formula, for example
 * `(2 * base + iv + ev / 4) * level / 100 + 5`, into a program of a few bytes.
 * core::Formula checks the program and runs it. It does not allocate memory and
 * it cannot crash: a program that is not correct is refused when it is set.
 *
 * A program is a list of instructions for a stack of numbers (signed, 64 bits).
 *
 * | Byte | Instruction | Effect |
 * |---|---|---|
 * | 0x01 | Constant | Pushes the next 4 bytes (a signed number, little-endian). |
 * | 0x02 | Variable | Pushes the variable with the number in the next byte. |
 * | 0x10 | Add | Replaces the two top numbers with their sum. |
 * | 0x11 | Subtract | Replaces `a`, `b` (b on top) with `a - b`. |
 * | 0x12 | Multiply | Replaces the two top numbers with their product. |
 * | 0x13 | Divide | Replaces `a`, `b` with `a / b`, rounded toward 0. A division by 0 gives 0. |
 * | 0x14 | Remainder | Replaces `a`, `b` with the remainder of `a / b`. A division by 0 gives 0. |
 * | 0x15 | Negate | Changes the sign of the top number. |
 * | 0x16 | Minimum | Replaces the two top numbers with the smaller one. |
 * | 0x17 | Maximum | Replaces the two top numbers with the bigger one. |
 * | 0x18 | Equal | Replaces `a`, `b` with 1 when they are equal, else 0. |
 * | 0x19 | Less | Replaces `a`, `b` with 1 when `a < b`, else 0. |
 * | 0x1A | Greater | Replaces `a`, `b` with 1 when `a > b`, else 0. |
 * | 0x1B | Choose | Replaces `c`, `a`, `b` (b on top) with `a` when `c` is not 0, else `b`. |
 *
 * The program must leave exactly one number on the stack.
 */

#pragma once

#include "core/types.h"

namespace core {

/// A program of a formula of integers.
class Formula {
public:
  /// The most bytes of a program.
  static constexpr u32 kMaxSize = 120;
  /// The most numbers on the stack.
  static constexpr u32 kStackSize = 16;
  /// The most variables that a program can read.
  static constexpr u32 kMaxVariables = 8;

  /// Gives the program. Returns false (and keeps the old program) when the
  /// program is not correct: an unknown instruction, a variable that does not
  /// exist, a stack that is too small or too big, or a result that is not one
  /// number.
  bool Set(const u8* bytes, u32 size);

  /// Removes the program: IsSet() is false.
  void Clear() { size_ = 0; }

  /// Returns true when the formula has a program.
  bool IsSet() const { return size_ != 0; }

  /// Runs the program. `variables` has `count` numbers; a variable that is
  /// not in the list is 0.
  s64 Evaluate(const s32* variables, u32 count) const;

private:
  u8 code_[kMaxSize] = {};
  u32 size_ = 0;
};

} // namespace core
