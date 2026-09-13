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

#include "common.h"

namespace core {

/**
 * @brief Handles function redirection (Hooking) via instruction overwriting.
 * This class implements an INLINE hook by replacing function headers with
 * an absolute jump. It uses a gateway (trampoline) to execute the original
 * logic.
 */
class Hook {
public:
  Hook() = default;

  void Initialize(u32 src, u32 dst);
  void Enable(bool force = false);
  void Disable();

  INLINE bool IsEnabled() const { return is_enabled_; }
  INLINE bool IsInitialized() const { return is_initialized_; }
  INLINE void Clear() {
    is_enabled_ = false;
    is_initialized_ = false;
  }

  /**
   * @brief Calls the original, unhooked function through the gateway.
   * @tparam R    The return type of the original function.
   * @tparam Args The argument types of the original function.
   * @param args  The arguments to pass to the original function.
   * @return      The result of the original function call.
   */
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
