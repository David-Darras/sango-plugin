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
 * @file hook.cc
 * @brief Installs and removes the hooks. See core/hook.h.
 */

#include "core/hook.h"

#include "core/memory.h"

namespace core {
namespace {

// Returns true when the memory zone of the address is writable.
bool IsWritable(u32 address) {
  MemInfo info;
  PageInfo page;
  if (R_FAILED(svcQueryMemory(&info, &page, address))) return false;
  return (info.perm & MEMPERM_WRITE) != 0;
}

} // namespace

HookBase* HookBase::first_ = nullptr;

HookBase::HookBase(u32 address, u32 function, bool has_process, uptr process)
    : address_(address),
      function_(function),
      process_(process),
      has_process_(has_process),
      is_automatic_(true) {
  Link();
}

void HookBase::Link() {
  if (is_linked_) return;
  next_ = first_;
  first_ = this;
  is_linked_ = true;
}

void HookBase::Install(u32 address, u32 function, bool has_process,
                       uptr process) {
  if (address == 0) return; // The address is not known for this game yet.
  if (is_enabled_) return;
  address_ = address;
  function_ = function;
  has_process_ = has_process;
  process_ = process;
  if (has_process) {
    Link();
  } else {
    Enable();
  }
}

void HookBase::Enable(bool force) {
  if (address_ == 0) return; // The address is not known for this game yet.
  if (!force && is_enabled_) return;

  // The code of a CRO is read-only.
  const bool unlock = has_process_ && !IsWritable(address_);
  if (unlock) MemoryManager::ToggleProtection(address_, true);

  if (!is_initialized_) {
    original_code_[0] = READ32(address_);
    original_code_[1] = READ32(address_ + 4);
    svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (uptr)original_code_, 8);

    gateway_[0] = READ32(address_);
    gateway_[1] = READ32(address_ + 4);
    gateway_[2] = 0xE51FF004; // ldr pc, [pc, #-4]
    gateway_[3] = address_ + 8;
    svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (uptr)gateway_, 0x10);

    is_initialized_ = true;
  }

  svcInvalidateEntireInstructionCache();

  WRITE32(address_, 0xE51FF004); // ldr pc, [pc, #-4]
  WRITE32(address_ + 4, function_); // .word function
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, address_, 8);

  if (unlock) MemoryManager::ToggleProtection(address_, false);
  is_enabled_ = true;
}

void HookBase::Disable() {
  if (!is_enabled_) return;

  const bool unlock = has_process_ && !IsWritable(address_);
  if (unlock) MemoryManager::ToggleProtection(address_, true);

  svcInvalidateEntireInstructionCache();

  WRITE32(address_, original_code_[0]);
  WRITE32(address_ + 4, original_code_[1]);
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, address_, 8);

  if (unlock) MemoryManager::ToggleProtection(address_, false);
  is_enabled_ = false;
}

void HookBase::InstallAll() {
  for (HookBase* hook = first_; hook != nullptr; hook = hook->next_) {
    if (hook->is_automatic_ && !hook->has_process_) hook->Enable();
  }
}

void HookBase::OnProcessLoad(uptr vtable) {
  // An unknown vtable reads 0. It never matches.
  if (vtable == 0) return;
  for (HookBase* hook = first_; hook != nullptr; hook = hook->next_) {
    if (hook->has_process_ && hook->process_ == vtable) hook->Enable(true);
  }
}

} // namespace core
