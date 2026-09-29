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

#include "common.h"
#include "ui/address.h"

namespace ui {
class PartySelect {
  MAKE_SINGLETON(PartySelect)

public:
  enum class Status : u8 {
    kUnsupported,
    kCancelled,
    kSelected,
  };

  static constexpr u32 kMaxMembers = 6;
  static constexpr bool kIsSupported = address::kCallPokemonList != 0;

  void Arm(u32 count);
  Status GetResult(u8* order);

  static void Initialize();

private:
  static constexpr u32 kBufferSize = 32;
  static constexpr u32 kContextScanStart = 0x20;
  static constexpr u32 kContextScanEnd = 0x48;

  static void CallHook(void* process_manager, u8* context, u8* result);

  u32 count_ = 0;
  bool is_armed_ = false;
  bool is_captured_ = false;
  alignas(4) u8 result_[kBufferSize] = {};
};
} // namespace ui