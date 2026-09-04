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
  global_data::Move::GetInstance(MoveID::kAcupressure).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kDragonDance).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kSwordsDance).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kNastyPlot).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kAgility).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kRockPolish).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kMinimize).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kDoubleTeam).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kBulkUp).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kCalmMind).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kQuiverDance).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kAmnesia).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kTailGlow).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kCottonGuard).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kAcidArmor).base_pp = 1;
  global_data::Move::GetInstance(MoveID::kStockpile).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kCurse).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kSubstitute).base_pp = 2;
  global_data::Move::GetInstance(MoveID::kProtect).base_pp = 2;
}
} // namespace kaizo