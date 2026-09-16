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
 * along with this program.  If not, see <https:
 */

#include "pokemon/native/move_data.h"
#include "pokemon/constant/move.h"

namespace kaizo {
void PatchMoveData() {
  pokemon::MoveData::GetInstance(MoveId::kAcupressure).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kDragonDance).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kSwordsDance).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kNastyPlot).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kAgility).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kRockPolish).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kMinimize).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kDoubleTeam).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kBulkUp).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kCalmMind).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kQuiverDance).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kAmnesia).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kTailGlow).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kCottonGuard).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kAcidArmor).base_pp = 1;
  pokemon::MoveData::GetInstance(MoveId::kStockpile).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kCurse).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kSubstitute).base_pp = 2;
  pokemon::MoveData::GetInstance(MoveId::kProtect).base_pp = 2;
}
} // namespace kaizo
