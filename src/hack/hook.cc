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

#include "hack/hook.h"

Hook::Hook() : src_addr_(0), dst_addr_(0), is_enabled_(false) {
  // Explicitly initialize members to safe defaults for the empty array
}

void Hook::Initialize(u32 src, u32 dst) {
  src_addr_ = src;
  dst_addr_ = dst;

  // Save the original first two instructions (8 bytes) of the source function
  original_code_[0] = READ(vu32, src);
  original_code_[1] = READ(vu32, src + 4);
  svcFlushProcessDataCache(0xFFFF8001, (uptr)original_code_, 8);

  // Setup the gateway to allow calling the original function
  gateway_[0] = READ(vu32, src);
  gateway_[1] = READ(vu32, src + 4);
  gateway_[2] = 0xE51FF004;  // ARM opcode for: ldr pc, [pc, #-4]
  gateway_[3] = src + 8;  // Address to jump back to, skipping the hooked bytes
  svcFlushProcessDataCache(0xFFFF8001, (uptr)gateway_, 0x10);

  Enable();
}

void Hook::Enable() {
  if (is_enabled_) return;

  // Flush caches to prevent CPU from executing stale, cached instructions
  svcInvalidateEntireInstructionCache();

  // Overwrite the start of the function with an absolute jump to our
  // destination
  WRITE(vu32, src_addr_, 0xE51FF004);     // ldr pc, [pc, #-4]
  WRITE(vu32, src_addr_ + 4, dst_addr_);  // .word dst
  svcFlushProcessDataCache(0xFFFF8001, src_addr_, 8);

  is_enabled_ = true;
}

void Hook::Disable() {
  if (!is_enabled_) return;

  // Flush caches to prevent CPU from executing stale, cached instructions
  svcInvalidateEntireInstructionCache();

  // Restore the original instructions
  WRITE(vu32, src_addr_, original_code_[0]);
  WRITE(vu32, src_addr_ + 4, original_code_[1]);
  svcFlushProcessDataCache(0xFFFF8001, src_addr_, 8);

  is_enabled_ = false;
}
