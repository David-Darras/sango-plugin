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

#include "hack/hook_manager.h"

HookManager HookManager::instance_ = HookManager();

void HookManager::Add(HookID id, u32 src, u32 dst) {
  if (id >= HookID::kMax) return;
  hooks_[(u32)id].Initialize(src, dst);
  count_++;
}

Hook *HookManager::Get(HookID id) {
  if (id >= HookID::kMax) return nullptr;
  return &hooks_[(u32)id];
}
