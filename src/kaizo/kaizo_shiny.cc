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

#include "common.h"
#include "feature/hook_manager.h"

namespace kaizo {
static u32 ToShinyHook(u32 id, u32 pid) {
  return 0b010;
}

static u32 ToNormalHook(u32 id, u32 pid) {
  return 0b101;
}

// probability : 1/8
static bool IsShinyHook(u32 id, u32 pid) {
  return (pid & 0b111) == 0b010;
}

void InitializeShinyHook() {
  HookManager::Initialize(HookID::kIsShiny, ADDRESS_IS_SHINY,
                          (uptr)IsShinyHook);
  HookManager::Initialize(HookID::kFromNormalToShiny,
                          ADDRESS_POKEMON_UTILS_TO_SHINY,
                          (uptr)ToShinyHook);
  HookManager::Initialize(HookID::kFromShinyToNormal,
                          ADDRESS_POKEMON_UTILS_TO_NORMAL,
                          (uptr)ToNormalHook);
}
}