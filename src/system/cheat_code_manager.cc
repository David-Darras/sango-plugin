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

#include "system/cheat_code_manager.h"

CheatCodeManager CheatCodeManager::instance_ = CheatCodeManager();

void CheatCodeManager::Add(CheatCodeId id, callback_t callback,
                           void* args) {
  if (id >= CheatCodeId::kMax) return;
  cheat_codes_[(u32)id].Initialize(callback, args);
  count_++;
}

CheatCode* CheatCodeManager::Get(CheatCodeId id) {
  if (id >= CheatCodeId::kMax) return nullptr;
  return &cheat_codes_[(u32)id];
}

void CheatCodeManager::Update() {
  for (u32 i = 0; i < kMaxCheatCodes; ++i) {
    if (cheat_codes_[i].IsEnabled()) {
      cheat_codes_[i].Run();
    }
  }
}
