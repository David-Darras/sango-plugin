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

#include "feature/cheat_code.h"

enum class CheatCodeId {
  kNoclip,
  kSwarmMod,
  kNoEncounter,
  kMax
};

class CheatCodeManager {
  MAKE_SINGLETON(CheatCodeManager)

public:
  void Add(CheatCodeId id, cheat_code_callback_t on_enable,
           cheat_code_callback_t on_disable,
           bool do_each_frame) {
    if (id >= CheatCodeId::kMax) return;
    cheat_codes_[(u32)id].Initialize(on_enable, on_disable, do_each_frame);
    count_++;
  }

  CheatCode* Get(CheatCodeId id) {
    if (id >= CheatCodeId::kMax) return nullptr;
    return &cheat_codes_[(u32)id];
  }

  void Update() const {
    for (u32 i = 0; i < kMaxCheatCodes; ++i) {
      if (cheat_codes_[i].DoEachFrame()) {
        cheat_codes_[i].Execute();
      }
    }
  }

  STATIC_INLINE void Initialize(CheatCodeId id, cheat_code_callback_t on_enable,
                                cheat_code_callback_t on_disable,
                                bool do_each_frame) {
    GetInstance().Add(id, on_enable, on_disable, do_each_frame);
  }

private:
  static constexpr u32 kMaxCheatCodes = (u32)CheatCodeId::kMax;

  CheatCode cheat_codes_[kMaxCheatCodes];
  u32 count_ = 0;
};