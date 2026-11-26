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
 * @file field_move.cc
 * @brief Uses the field moves from the menu.
 *
 * The declarations are in overworld/patch/field_move.h.
 */

#include "overworld/patch/field_move.h"
#include "overworld/native/map_manager.h"
#include "ui/main_application.h"
#include "core/address.h"
#include "overworld/address.h"

namespace overworld {

namespace {
/// The instruction of the game that checks if Fly works here.
u32 fly_check = 0;
} // namespace

void FieldMove::Initialize() {
  if (address::kFlyCheck) fly_check = READ32(address::kFlyCheck);
}

void FieldMove::PatchFlyCheck() {
  if (address::kFlyCheck == 0 || fly_check == 0) return;
  // NOP in place of the check, or the instruction of the game again.
  const u32 wanted = GetInstance().fly_anywhere ? 0xE1A00000 : fly_check;
  // Nothing is written as long as the setting stays off.
  if (READ32(address::kFlyCheck) == wanted) return;
  svcInvalidateEntireInstructionCache();
  WRITE32(address::kFlyCheck, wanted);
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, address::kFlyCheck, 4);
}

void FieldMove::Execute(FieldMoveId move) {
  auto& main_app = ui::MainApplication::GetInstance();
  if (main_app.CheckProcess(address::kVtable)) return;

  auto& map_manager = MapManager::GetInstance();

  struct {
    u16 zone_id;
    u16 team_index;
    MapManager* map_manager;
  } context = {8, 0, &map_manager};

  core::GameFunction<void(void*, FieldMoveId)>::Call(
      core::address::kDoFieldMove, &context, move);

  ui::MainApplication::GetInstance().ForceClose();
}

} // namespace overworld
