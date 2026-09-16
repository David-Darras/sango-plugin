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

#include <types.h>

namespace script {

enum class WindowType : u16 {
  kTalk = 0,
  kTalkVariable = 1,
  kJagged = 2,
  kJaggedVariable = 3,
  kFluffyVariable = 4,
  kTown = 5,
  kFacility = 6,
  kBargain = 7,
  kISee = 8,
  kRoadSign = 9,
  kSign = 10,
  kSystem = 11,
  kSmallCircle = 12,
  kMoney = 13,
};

} // namespace script
