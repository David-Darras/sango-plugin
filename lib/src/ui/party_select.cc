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
 * @file party_select.cc
 * @brief Lets a script select several Pokémon of the party.
 *
 * The declarations are in ui/patch/party_select.h.
 */

#include "ui/patch/party_select.h"

#include "core/hook_manager.h"

namespace ui {

void PartySelect::Initialize() {
  if (!kIsSupported) return;
  core::HookManager::Initialize(HookId::kCallPokemonList,
                                address::kCallPokemonList,
                                (uptr)CallHook);
}

void PartySelect::Arm(u32 count) {
  count_ = count;
  is_armed_ = true;
  is_captured_ = false;
  for (u32 i = 0; i < kBufferSize; i++) result_[i] = 0xFF;
}

void PartySelect::CallHook(void* process_manager, u8* context, u8* result) {
  auto& feat = GetInstance();
  if (feat.is_armed_) {
    feat.is_armed_ = false;

    for (u32 i = kContextScanStart; i < kContextScanEnd; i++) {
      if (context[i] == 1 && context[i + 1] == 1 && context[i + 2] == 0 &&
          context[i + 3] == 1) {
        context[i] = (u8)feat.count_;
        context[i + 1] = (u8)feat.count_;
        result = feat.result_;
        feat.is_captured_ = true;
        break;
      }
    }
  }
  core::HookManager::Call<void>(HookId::kCallPokemonList, process_manager,
                                context, result);
}

PartySelect::Status PartySelect::GetResult(u8* order) {
  is_armed_ = false;
  if (!kIsSupported || !is_captured_) return Status::kUnsupported;
  is_captured_ = false;

  bool is_written = false;
  for (u32 i = 0; i < kBufferSize; i++) {
    if (result_[i] != 0xFF) is_written = true;
  }
  if (!is_written) return Status::kUnsupported;

  const s8* bytes = (const s8*)result_;
  for (s32 start = kBufferSize - kMaxMembers; start >= 0; start--) {
    if (bytes[start] < 0 || bytes[start] >= (s8)kMaxMembers) continue;

    u8 picked[kMaxMembers];
    u32 found = 0;
    bool is_valid = true;
    for (u32 i = 0; i < kMaxMembers && is_valid; i++) {
      const s8 slot = bytes[start + i];
      if (slot == -1) continue;
      if (slot < 0 || slot >= (s8)kMaxMembers) {
        is_valid = false;
        break;
      }
      for (u32 j = 0; j < found; j++) {
        if (picked[j] == (u8)slot) is_valid = false;
      }
      if (is_valid && found < kMaxMembers) picked[found++] = (u8)slot;
    }
    if (!is_valid || found != count_) continue;

    for (u32 i = 0; i < found; i++) order[i] = picked[i];
    return Status::kSelected;
  }
  return Status::kCancelled;
}

} // namespace ui
