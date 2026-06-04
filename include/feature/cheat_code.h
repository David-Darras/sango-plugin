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

#ifndef SANGO_PLUGIN_CHEAT_CODE_H
#define SANGO_PLUGIN_CHEAT_CODE_H

#include "common.h"

class CheatCode {
public:
  void Initialize(cheat_code_callback_t on_enable,
                  cheat_code_callback_t on_disable,
                  bool do_each_frame) {
    is_enabled_ = false;
    do_each_frame_ = do_each_frame;
    on_enable_ = on_enable;
    on_disable_ = on_disable;
  }

  INLINE void Toggle() { is_enabled_ = !is_enabled_; }

  INLINE bool IsEnabled() const { return is_enabled_; }

  INLINE bool DoEachFrame() const { return do_each_frame_; }

  INLINE void Execute() const {
    if (is_enabled_) {
      if (on_enable_) {
        on_enable_();
      }
    } else {
      if (on_disable_) {
        on_disable_();
      }
    }
  }

private:
  bool is_enabled_ = false;
  bool do_each_frame_ = false;
  cheat_code_callback_t on_enable_ = nullptr;
  cheat_code_callback_t on_disable_ = nullptr;
};

#endif  // SANGO_PLUGIN_CHEAT_CODE_H