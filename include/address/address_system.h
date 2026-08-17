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

/** * @name System, Entrypoint & Standard Library Addresses
 * @{ */
#define ADDRESS_STRING_VTABLE (0x5DE3BC)
#define ADDRESS_ENTRYPOINT (0x00122938) // Render Home Button Blocked
#define ADDRESS_GAME_MANAGER (0x08C69094)
#define ADDRESS_CALL_APP (0x003CFBA8)
#define ADDRESS_OS_READ_ONLY_KERNEL_INFO (0x1FF80000)
#define ADDRESS_HEAP_TABLE (0x08000000)
#define ADDRESS_STD_WCSLEN \
  (0x001003E8 | 1)  ///< Standard wcslen function (Thumb mode)
#define ADDRESS_STD_VSWPRINTF \
  (0x001004B8 | 1)         ///< Standard vswprintf function (Thumb mode)
#define BUFFER_SIZE (128)  ///< Default buffer size for formatted strings
#define ADDRESS_GET_RANDOM_VALUE (0x0048AF80)
#define ADDRESS_GET_ELAPSED_TIME (0x0012B724)
#define ADDRESS_CONVERT_TIME_TO_SECONDS (0x0012FBDC)
#define ADDRESS_DATE_TIME (0x006174F8)
#define ADDRESS_CHECK_APP_REQUEST (0x007BDE50)
#define ADDRESS_MAIN_PROCESS_LOOP (0x003AB62C)
/** @} */
