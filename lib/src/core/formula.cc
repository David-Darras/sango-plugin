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
 * @file formula.cc
 * @brief A small calculation that Sango Studio writes: a formula of integers.
 *
 * The declarations and the instructions are in core/formula.h.
 */

#include "core/formula.h"

namespace core {

namespace {

enum Op : u8 {
  kConstant = 0x01,
  kVariable = 0x02,
  kAdd = 0x10,
  kSubtract = 0x11,
  kMultiply = 0x12,
  kDivide = 0x13,
  kRemainder = 0x14,
  kNegate = 0x15,
  kMinimum = 0x16,
  kMaximum = 0x17,
  kEqual = 0x18,
  kLess = 0x19,
  kGreater = 0x1A,
  kChoose = 0x1B,
};

/// How many numbers an instruction takes from the stack and gives back.
/// Returns false for an unknown instruction.
bool StackEffect(u8 op, u32* pops, u32* pushes) {
  switch (op) {
  case kConstant:
  case kVariable:
    *pops = 0;
    *pushes = 1;
    return true;
  case kNegate:
    *pops = 1;
    *pushes = 1;
    return true;
  case kAdd:
  case kSubtract:
  case kMultiply:
  case kDivide:
  case kRemainder:
  case kMinimum:
  case kMaximum:
  case kEqual:
  case kLess:
  case kGreater:
    *pops = 2;
    *pushes = 1;
    return true;
  case kChoose:
    *pops = 3;
    *pushes = 1;
    return true;
  default:
    return false;
  }
}

/// The size of an instruction in bytes.
u32 InstructionSize(u8 op) {
  if (op == kConstant) return 5;
  if (op == kVariable) return 2;
  return 1;
}

/// Turns the bits of an unsigned number into a signed number (the calculations
/// wrap around, they never overflow).
s64 Wrap(u64 value) { return static_cast<s64>(value); }

s32 ReadS32(const u8* at) {
  const u32 value = static_cast<u32>(at[0]) | (static_cast<u32>(at[1]) << 8) |
                    (static_cast<u32>(at[2]) << 16) |
                    (static_cast<u32>(at[3]) << 24);
  return static_cast<s32>(value);
}

} // namespace

bool Formula::Set(const u8* bytes, u32 size) {
  if (bytes == nullptr || size == 0 || size > kMaxSize) return false;
  u32 depth = 0;
  u32 at = 0;
  while (at < size) {
    const u8 op = bytes[at];
    u32 pops = 0;
    u32 pushes = 0;
    if (!StackEffect(op, &pops, &pushes)) return false;
    const u32 length = InstructionSize(op);
    if (at + length > size) return false;
    if (op == kVariable && bytes[at + 1] >= kMaxVariables) return false;
    if (depth < pops) return false;
    depth = depth - pops + pushes;
    if (depth > kStackSize) return false;
    at += length;
  }
  if (depth != 1) return false;
  for (u32 i = 0; i < size; i++) code_[i] = bytes[i];
  size_ = size;
  return true;
}

s64 Formula::Evaluate(const s32* variables, u32 count) const {
  s64 stack[kStackSize];
  u32 depth = 0;
  u32 at = 0;
  while (at < size_) {
    const u8 op = code_[at];
    switch (op) {
    case kConstant:
      stack[depth++] = ReadS32(code_ + at + 1);
      break;
    case kVariable: {
      const u32 index = code_[at + 1];
      stack[depth++] = (variables != nullptr && index < count) ? variables[index] : 0;
      break;
    }
    case kNegate:
      stack[depth - 1] = Wrap(0u - static_cast<u64>(stack[depth - 1]));
      break;
    case kChoose: {
      const s64 otherwise = stack[depth - 1];
      const s64 then = stack[depth - 2];
      const s64 condition = stack[depth - 3];
      depth -= 2;
      stack[depth - 1] = condition != 0 ? then : otherwise;
      break;
    }
    default: {
      const s64 b = stack[depth - 1];
      const s64 a = stack[depth - 2];
      depth--;
      s64 result = 0;
      switch (op) {
      case kAdd: result = Wrap(static_cast<u64>(a) + static_cast<u64>(b)); break;
      case kSubtract: result = Wrap(static_cast<u64>(a) - static_cast<u64>(b)); break;
      case kMultiply: result = Wrap(static_cast<u64>(a) * static_cast<u64>(b)); break;
      case kDivide: result = b == 0 ? 0 : (b == -1 ? Wrap(0u - static_cast<u64>(a)) : a / b); break;
      case kRemainder: result = (b == 0 || b == -1) ? 0 : a % b; break;
      case kMinimum: result = a < b ? a : b; break;
      case kMaximum: result = a > b ? a : b; break;
      case kEqual: result = a == b ? 1 : 0; break;
      case kLess: result = a < b ? 1 : 0; break;
      case kGreater: result = a > b ? 1 : 0; break;
      default: break;
      }
      stack[depth - 1] = result;
      break;
    }
    }
    at += InstructionSize(op);
  }
  return depth == 1 ? stack[0] : 0;
}

} // namespace core
