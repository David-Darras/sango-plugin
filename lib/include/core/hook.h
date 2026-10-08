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
 * @file hook.h
 * @brief One hook: the redirection of one function of the game.
 */

#pragma once

#include "common.h"

namespace core {

/**
 * @brief Redirects one function of the game to a function of the plugin.
 *
 * The hook replaces the first two instructions of the game function with a
 * jump to the plugin function. A gateway keeps the two instructions and a
 * jump back: CallOriginal() uses it to run the original function.
 *
 * Use core::HookManager instead of this class.
 *
 * @see docs/concepts/hooks-and-addresses.md
 */
class Hook {
public:
  Hook() = default;

  /// Sets the game function (`src`) and the plugin function (`dst`).
  void Initialize(u32 src, u32 dst);
  /// Writes the jump. With `force`, writes it again also when it is enabled.
  void Enable(bool force = false);
  /// Writes the original instructions back.
  void Disable();

  /// Returns true when the jump is in the game code.
  INLINE bool IsEnabled() const { return is_enabled_; }
  /// Returns true when the gateway exists.
  INLINE bool IsInitialized() const { return is_initialized_; }
  /// Forgets the state. Use it when the game removed the code (a CRO).
  INLINE void Clear() {
    is_enabled_ = false;
    is_initialized_ = false;
  }

  /// Runs the original function with the gateway.
  template <typename R, typename... Args>
  R CallOriginal(Args... args) {
    using FunctionType = R (*)(Args...);
    FunctionType func = reinterpret_cast<FunctionType>(&gateway_[0]);
    return func(args...);
  }

private:
  bool is_enabled_ = false;
  u32 src_addr_ = 0;
  u32 dst_addr_ = 0;
  u32 original_code_[2];
  u32 gateway_[4];
  bool is_initialized_ = false;
};

} // namespace core
