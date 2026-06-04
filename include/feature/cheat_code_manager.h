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

#ifndef SANGO_PLUGIN_CHEAT_CODE_MANAGER_H
#define SANGO_PLUGIN_CHEAT_CODE_MANAGER_H

#include "feature/cheat_code.h"

enum class CheatCodeId {
  kInstantEggHatch,
  kInstantEggGeneration,
  kInstantMaxExpForDayCare,
  kNoclip,
  kSwarmMod,
  kMax
};

class CheatCodeManager {
public:
  STATIC_INLINE CheatCodeManager& GetInstance() { return instance_; }
  void Add(CheatCodeId id, cheat_code_callback_t on_enable,
           cheat_code_callback_t on_disable,
           bool do_each_frame);
  CheatCode* Get(CheatCodeId id);
  void Update();

  STATIC_INLINE void Initialize(CheatCodeId id, cheat_code_callback_t on_enable,
                                cheat_code_callback_t on_disable,
                                bool do_each_frame) {
    GetInstance().Add(id, on_enable, on_disable, do_each_frame);
  }

private:
  CheatCodeManager() : count_(0) {
  }

  static constexpr u32 kMaxCheatCodes = (u32)CheatCodeId::kMax;
  static CheatCodeManager instance_;

  CheatCode cheat_codes_[kMaxCheatCodes];
  u32 count_;
};

#endif  // SANGO_PLUGIN_CHEAT_CODE_MANAGER_H