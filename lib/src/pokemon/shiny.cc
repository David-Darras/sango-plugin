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

#include "pokemon/patch/shiny.h"

#include "core/hook_manager.h"

namespace pokemon {

void Shiny::Initialize() {
  core::HookManager::Initialize(HookId::kIsShiny, core::address::kIsShiny,
                          (uptr)IsShinyHook);
  core::HookManager::Initialize(HookId::kFromNormalToShiny,
                          address::kUtilsToShiny, (uptr)ToShinyHook);
  core::HookManager::Initialize(HookId::kFromShinyToNormal,
                          address::kUtilsToNormal, (uptr)ToNormalHook);
}

u32 Shiny::ToShinyHook(u32 id, u32 pid) {
  if (GetInstance().rate == ShinyRate::kOff)
    return core::HookManager::Call<u32>(HookId::kFromNormalToShiny, id, pid);
  return kForcedShiny;
}

u32 Shiny::ToNormalHook(u32 id, u32 pid) {
  if (GetInstance().rate == ShinyRate::kOff)
    return core::HookManager::Call<u32>(HookId::kFromShinyToNormal, id, pid);
  return kForcedNormal;
}

bool Shiny::IsShinyHook(u32 id, u32 pid) {
  const ShinyRate rate = GetInstance().rate;
  if (rate == ShinyRate::kOff)
    return core::HookManager::Call<bool>(HookId::kIsShiny, id, pid);

  if (pid == kForcedShiny) return true;
  if (pid == kForcedNormal) return false;

  // k1_1 keeps no bit, k1_2 one bit, k1_4 two...
  const u32 bits = static_cast<u32>(rate) - 1u;
  const u32 mask = (1u << bits) - 1u;
  return (pid & mask) == 0u;
}

} // namespace pokemon
