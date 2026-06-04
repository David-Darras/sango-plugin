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
#include "cheat_code_manager.h"
#include "common.h"
#include "core/core.h"

namespace feature {
class DayCareCheatCode {
  MAKE_SINGLETON(DayCareCheatCode)
  STATIC_INLINE void Initialize() {
    CheatCodeManager::Initialize(CheatCodeId::kInstantEggHatch,
                                 [] {
                                   WRITE(u32, ADDRESS_EGG_HATCH, 0xEA000007);
                                 }, [] {
                                   WRITE(u32, ADDRESS_EGG_HATCH, 0x0A000007);
                                 }, false
        );
    CheatCodeManager::Initialize(CheatCodeId::kInstantEggGeneration,
                                 [] {
                                   WRITE(u32, ADDRESS_EGG_GENERATION,
                                         0xE1A00000);
                                 }, [] {
                                   WRITE(u32, ADDRESS_EGG_GENERATION,
                                         0x2A000002);
                                 }, false
        );
    CheatCodeManager::Initialize(CheatCodeId::kInstantMaxExpForDayCare,
                                 [] {
                                   WRITE(u32, ADDRESS_DAY_CARE_MAX_EXP,
                                         0x15824004);
                                   WRITE(u32, ADDRESS_DAY_CARE_MAX_EXP + 0x20,
                                         0x158240F4);
                                 }, [] {
                                   WRITE(u32, ADDRESS_DAY_CARE_MAX_EXP,
                                         0x1582C004);
                                   WRITE(u32, ADDRESS_DAY_CARE_MAX_EXP + 0x20,
                                         0x1582C0F4);
                                 }, false
        );
  }
};
} // namespace feature