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

#include "game/global_data/move.h"
#include "game/constant/move.h"

namespace kaizo {
void PatchMoveData() {
  global_data::Move::GetInstance(MoveId::kAcupressure).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kDragonDance).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kSwordsDance).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kNastyPlot).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kAgility).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kRockPolish).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kMinimize).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kDoubleTeam).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kBulkUp).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kCalmMind).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kQuiverDance).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kAmnesia).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kTailGlow).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kCottonGuard).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kAcidArmor).base_pp = 1;
  global_data::Move::GetInstance(MoveId::kStockpile).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kCurse).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kSubstitute).base_pp = 2;
  global_data::Move::GetInstance(MoveId::kProtect).base_pp = 2;
}
} // namespace kaizo