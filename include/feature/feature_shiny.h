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
#include "feature/hook_manager.h"

namespace feature {

struct Shiny {
  MAKE_SINGLETON(Shiny)
  enum Rate : u32 {
    kOff,
    k1_1,
    k1_2,
    k1_4,
    k1_8,
    k1_16,
    k1_32,
    k1_64,
    k1_128,
    k1_256,
    k1_512,
    k1_1024,
    k1_2048,
    k1_4096,
    k1_8192,  
    k1_16384, 
    k1_32768, 
    k1_65536, 
    k1_131072,
    k1_262144,
    k1_524288,
    k1_1048576,
    kRateMax,
  };

  enum : u32 { kForcedShiny = 0xFFFFFFFEu, kForcedNormal = 0xFFFFFFFFu };

  u32 rate = kOff;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kIsShiny, ADDRESS_IS_SHINY,
                            (uptr)IsShinyHook);
    HookManager::Initialize(HookID::kFromNormalToShiny,
                            ADDRESS_POKEMON_UTILS_TO_SHINY,
                            (uptr)ToShinyHook);
    HookManager::Initialize(HookID::kFromShinyToNormal,
                            ADDRESS_POKEMON_UTILS_TO_NORMAL,
                            (uptr)ToNormalHook);
  }

  static u32 ToShinyHook(u32 id, u32 pid) {
    if (GetInstance().rate == kOff)
      return HookManager::Call<u32>(HookID::kFromNormalToShiny, id, pid);
    return kForcedShiny;
  }

  static u32 ToNormalHook(u32 id, u32 pid) {
    if (GetInstance().rate == kOff)
      return HookManager::Call<u32>(HookID::kFromShinyToNormal, id, pid);
    return kForcedNormal;
  }

  static bool IsShinyHook(u32 id, u32 pid) {
    const u32 rate = GetInstance().rate;
    if (rate == kOff)
      return HookManager::Call<bool>(HookID::kIsShiny, id, pid);

    if (pid == kForcedShiny) return true;
    if (pid == kForcedNormal) return false;

    const u32 mask = (1u << (rate - 1u)) - 1u;
    return (pid & mask) == 0u;
  }
};
} // namespace feature