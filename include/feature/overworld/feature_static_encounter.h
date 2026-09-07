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
#include "utils.h"
#include "feature/core/hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/species.h"
#include "game/core/manager.h"
#include "game/overworld/static_encounter.h"

namespace feature {
class StaticEncounter {
  MAKE_SINGLETON(StaticEncounter)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kCallStaticEncounter, 0x0077D0C4,
                            (uptr)CallStaticEncounterHook, false);
  }

  STATIC_INLINE void PatchLoad() {
    HookManager::ForceEnable(HookID::kCallStaticEncounter);
  }

private:
  static void RandomizeSpecies(u32 index) {
    auto& entry = overworld::StaticEncounter::GetInstance(index);
    entry.species = static_cast<Species>
        (1 + Utils::GetRandomValue(static_cast<u32>(Species::kCount) - 1));
    entry.form = Form::kNormal;
    ui::LogApplication::Print(u"static[%u]=%u", index, entry.species);
  }

  static s32 CallStaticEncounterHook(game::Manager* man, u32 index, u32 p3,
                                     u32 p4) {
    RandomizeSpecies(index);
    return HookManager::Call<s32>(HookID::kCallStaticEncounter, man, index,
                                  p3, p4);
  }
};
} // namespace feature