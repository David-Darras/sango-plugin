// /*
//  * Copyright (C) 2026  David Darras
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  *

#ifndef SANGO_PLUGIN_CHEAT_CODE_H
#define SANGO_PLUGIN_CHEAT_CODE_H

#include "common.h"

enum class CheatCodeId { kNoclip, kSwarmMod, kMax };

class CheatCode {
 public:
  CheatCode() : is_enabled_(false), callback_(nullptr), args_(nullptr) {}

  void Initialize(callback_t callback, void* args) {
    is_enabled_ = false;
    callback_ = callback;
    args_ = args;
  }

  void Toggle() { is_enabled_ = !is_enabled_; }

  bool IsEnabled() { return is_enabled_; }

  void Run() {
    if (callback_) callback_(args_);
  }

 private:
  bool is_enabled_;
  callback_t callback_;
  void* args_;
};

#endif  // SANGO_PLUGIN_CHEAT_CODE_H
