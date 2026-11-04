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
 * @file csvc.h
 * @brief The system calls of Luma3DS that the boot uses (see csvc.s).
 */

#pragma once

#include <3ds.h>

namespace boot {

/// The operations of svcControlProcess().
enum class ProcessOperation : u32 {
  /// Gives an event. The kernel signals it each time the memory layout of the
  /// process changes.
  kGetOnMemoryChangeEvent = 2,
};

} // namespace boot

extern "C" {

/// Does an operation on a process (Luma3DS).
Result svcControlProcess(Handle process, u32 operation, u32 argument_2,
                         u32 argument_3);

} // extern "C"
