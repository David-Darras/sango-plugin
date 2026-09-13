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

#include "overworld/patch/wild_encounter.h"

#include "core/cheat_code_manager.h"
#include "core/hook_manager.h"
#include "core/native/process_manager.h"
#include "overworld/native/map_manager.h"
#include "savedata/native/repel.h"

namespace overworld {

void WildEncounter::Initialize() {
  core::CheatCodeManager::Initialize(CheatCodeId::kNoEncounter, AddMaxRepel,
                               RemoveMaxRepel, true);
  core::HookManager::Initialize(HookId::kGetEncounterPokemon,
                          pokemon::address::kEncounterSetPokemon,
                          (uptr)GetEncounterPokemonHook, false);
  core::HookManager::Initialize(HookId::kGetNaviDexTable,
                          address::kGetNaviDexTable,
                          (uptr)GetNaviDexTable, true);
}

u16* WildEncounter::GetNaviDexTable(EncounterData* data, s32 data_size,
                                u32* count, void* heap,
                                u8 p4, bool p5) {
  if (data_size <= 0 || data == nullptr) return nullptr;

  auto& feat = GetInstance();
  if (feat.on_encounter_table != nullptr) feat.on_encounter_table(data);

  return core::HookManager::Call<u16*>(HookId::kGetNaviDexTable, data,
                                 data_size, count, heap, p4, p5);
}

void WildEncounter::AddMaxRepel() {
  savedata::Repel::GetInstance().spray_count = 100;
}

void WildEncounter::RemoveMaxRepel() {
  savedata::Repel::GetInstance().spray_count = 0;
}

bool WildEncounter::GetEncounterPokemonHook(u32 p0, u32 p1) {
  bool result = core::HookManager::Call<bool>(HookId::kGetEncounterPokemon, p0, p1);

  auto& feat = GetInstance();
  if (feat.on_wild_pokemon == nullptr) return result;
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(address::kVtable))
    return result;

  feat.on_wild_pokemon(MapManager::GetInstance().GetMap(), (PokemonData*)p0,
                       READ32(p0 + 108));

  return result;
}

} // namespace overworld
