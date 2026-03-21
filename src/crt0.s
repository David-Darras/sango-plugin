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

.section ".crt0"
.global _start, _exit, svcInvalidateEntireInstructionCache, svcFlushProcessDataCache
.align 4
.arm
.fpu vfp
.syntax unified

_start:
  stmfd sp!, {r0-r12, lr}
  vpush {d0-d15}
  ldr r0, =ADDRESS_TARGET-4
  ldr r0, [r0]
  cmp r0, #0
  bleq Initialize
  bl OnFrame
restore_context:
  vpop {d0-d15}
  ldmfd sp!, {r0-r12, lr}
  stmfd sp!, {r4, lr}
  ldr pc, =ADDRESS_SOURCE+4

_exit:
  bl Finalize
  ldr r0, =ADDRESS_SOURCE
  ldr r1, =0xE92D4010      @ stmfd sp!, {r4, lr}
  str r1, [r0]
  b restore_context

svcInvalidateEntireInstructionCache:
  svc 0x94
  bx lr

svcFlushProcessDataCache:
  svc 0x54
  bx lr
