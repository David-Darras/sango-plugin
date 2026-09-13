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

class CheatCode {
public:
  void Initialize(cheat_code_callback_t on_enable,
                  cheat_code_callback_t on_disable,
                  bool do_each_frame);

  INLINE void Toggle() { is_enabled_ = !is_enabled_; }
  INLINE bool IsEnabled() const { return is_enabled_; }
  INLINE bool DoEachFrame() const { return do_each_frame_; }
  void Execute() const;

private:
  bool is_enabled_ = false;
  bool do_each_frame_ = false;
  cheat_code_callback_t on_enable_ = nullptr;
  cheat_code_callback_t on_disable_ = nullptr;
};

} // namespace core
