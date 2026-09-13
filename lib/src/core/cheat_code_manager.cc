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

#include "core/cheat_code_manager.h"

namespace core {

void CheatCode::Initialize(cheat_code_callback_t on_enable,
                           cheat_code_callback_t on_disable,
                           bool do_each_frame) {
  is_enabled_ = false;
  do_each_frame_ = do_each_frame;
  on_enable_ = on_enable;
  on_disable_ = on_disable;
}

void CheatCode::Execute() const {
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

void CheatCodeManager::Add(CheatCodeId id, cheat_code_callback_t on_enable,
                           cheat_code_callback_t on_disable,
                           bool do_each_frame) {
  if (id >= CheatCodeId::kMax) return;
  cheat_codes_[(u32)id].Initialize(on_enable, on_disable, do_each_frame);
  count_++;
}

CheatCode* CheatCodeManager::Get(CheatCodeId id) {
  if (id >= CheatCodeId::kMax) return nullptr;
  return &cheat_codes_[(u32)id];
}

void CheatCodeManager::Update() const {
  for (u32 i = 0; i < kMaxCheatCodes; ++i) {
    if (cheat_codes_[i].DoEachFrame()) {
      cheat_codes_[i].Execute();
    }
  }
}

void CheatCodeManager::Initialize(CheatCodeId id, cheat_code_callback_t on_enable,
                                  cheat_code_callback_t on_disable,
                                  bool do_each_frame) {
  GetInstance().Add(id, on_enable, on_disable, do_each_frame);
}

} // namespace core
