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

#include "overworld/patch/static_randomizer.h"
#include "core/hook_manager.h"
#include "core/native/game_manager.h"
#include "overworld/native/static_encounter.h"
#include "ui/log_application.h"
#include "core/utils.h"

namespace overworld {

void StaticRandomizer::Initialize() {
  core::HookManager::Initialize(HookId::kCallStaticEncounter,
                          address::kCallStaticEncounter,
                          (uptr)CallStaticEncounterHook, false);
}

void StaticRandomizer::PatchLoad() {
  core::HookManager::ForceEnable(HookId::kCallStaticEncounter);
}

void StaticRandomizer::RandomizeSpecies(StaticEncounterId id) {
  auto& entry = StaticEncounter::GetInstance(id);
  entry.species = core::Utils::GetRandomEnum<SpeciesId>();
  entry.form = FormId::kNormal;
  ui::LogApplication::Print(u"static[%u]=%u", id, entry.species);
}

s32 StaticRandomizer::CallStaticEncounterHook(core::GameManager* man,
                                              StaticEncounterId id, u32 p3,
                                              u32 p4) {
  if (GetInstance().randomize_species) RandomizeSpecies(id);
  return core::HookManager::Call<s32>(HookId::kCallStaticEncounter, man, id, p3,
                                p4);
}

} // namespace overworld
