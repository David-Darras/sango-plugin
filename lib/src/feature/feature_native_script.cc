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

#include "feature/core/feature_native_script.h"

Coroutine* Coroutine::self_ = nullptr;

asm(R"(
  .text
  .align 2
  .global CoroutineSwitch
  .type CoroutineSwitch, %function
CoroutineSwitch:
  push  {r4-r11, lr}
  vpush {d8-d15}
  str   sp, [r0]
  mov   sp, r1
  vpop  {d8-d15}
  pop   {r4-r11, pc}
  .size CoroutineSwitch, .-CoroutineSwitch
)");
