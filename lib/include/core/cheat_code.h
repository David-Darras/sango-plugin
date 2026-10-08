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
 * @file cheat_code.h
 * @brief A cheat code: a function that the player enables in the menu.
 */

#pragma once

#include "common.h"

namespace core {

/**
 * @brief A cheat code: two functions and an on/off state.
 *
 * When the cheat code is enabled, it calls `on_enable`. When it is disabled,
 * it calls `on_disable`. It can call them at each frame.
 */
class CheatCode {
public:
  /// Sets the two functions. `do_each_frame`: call them at each frame.
  void Initialize(cheat_code_callback_t on_enable,
                  cheat_code_callback_t on_disable,
                  bool do_each_frame);

  /// Enables or disables the cheat code.
  INLINE void Toggle() { is_enabled_ = !is_enabled_; }
  /// Returns true when the cheat code is enabled.
  INLINE bool IsEnabled() const { return is_enabled_; }
  /// Returns true when the cheat code runs at each frame.
  INLINE bool DoEachFrame() const { return do_each_frame_; }
  /// Calls `on_enable` or `on_disable`, from the current state.
  void Execute() const;

private:
  bool is_enabled_ = false;
  bool do_each_frame_ = false;
  cheat_code_callback_t on_enable_ = nullptr;
  cheat_code_callback_t on_disable_ = nullptr;
};

} // namespace core
