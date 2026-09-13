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

#include "core/hook_manager.h"

namespace core {

void Hook::Initialize(u32 src, u32 dst) {
  src_addr_ = src;
  dst_addr_ = dst;
}

void Hook::Enable(bool force) {
  if (!force && is_enabled_) return;

  if (!is_initialized_) {
    original_code_[0] = READ32(src_addr_);
    original_code_[1] = READ32(src_addr_ + 4);
    svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (uptr)original_code_, 8);

    gateway_[0] = READ32(src_addr_);
    gateway_[1] = READ32(src_addr_ + 4);
    gateway_[2] = 0xE51FF004; // ARM opcode for: ldr pc, [pc, #-4]
    gateway_[3] = src_addr_ + 8;
    svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (uptr)gateway_, 0x10);

    is_initialized_ = true;
  }

  svcInvalidateEntireInstructionCache();

  WRITE32(src_addr_, 0xE51FF004); // ldr pc, [pc, #-4]
  WRITE32(src_addr_ + 4, dst_addr_); // .word dst
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, src_addr_, 8);

  is_enabled_ = true;
}

void Hook::Disable() {
  if (!is_enabled_) return;

  svcInvalidateEntireInstructionCache();

  WRITE32(src_addr_, original_code_[0]);
  WRITE32(src_addr_ + 4, original_code_[1]);
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, src_addr_, 8);

  is_enabled_ = false;
}

void HookManager::Add(HookId id, u32 src, u32 dst, bool enable) {
  if (id >= HookId::kMax) return;
  if (hooks_[(u32)id].IsEnabled()) return;
  hooks_[(u32)id].Initialize(src, dst);
  if (enable) {
    hooks_[(u32)id].Enable();
  }
  count_++;
}

Hook* HookManager::Get(HookId id) {
  if (id >= HookId::kMax) return nullptr;
  return &hooks_[(u32)id];
}

void HookManager::Initialize(HookId id, u32 src, u32 dst, bool enable) {
  GetInstance().Add(id, src, dst, enable);
}

void HookManager::Enable(HookId id) {
  GetInstance().Get(id)->Enable(false);
}

void HookManager::ForceEnable(HookId id) {
  GetInstance().Get(id)->Enable(true);
}

void HookManager::Disable(HookId id) {
  GetInstance().Get(id)->Disable();
}

void HookManager::Clear(HookId id) {
  GetInstance().Get(id)->Clear();
}

} // namespace core
