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

enum class ScriptVariable : u16 {
  kParam0 = 0x8000,
  kTemp0 = 0x8008,
  kReturn0 = 0x800C,
  kAnswer = 0x8010,
  kTalkTarget = 0x8011,
  kTalkStartTarget = 0x8012,
  kTalkOptions = 0x8013,
  kTargetBackground = 0x8014,
  kTargetPosition = 0x8015,
  kSmallMessageTail = 0x8016,
};

} // namespace script
