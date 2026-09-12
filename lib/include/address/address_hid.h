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

/** * @name HID (Human Interface Device) Addresses
 * @{ */
#define ADDRESS_DEVICE_GET_CONTROLLER (0x00117550)
#define ADDRESS_CONTROLLER_IS_KEY_PRESSED (0x004C86EC)
#define ADDRESS_CONTROLLER_IS_KEY_RELEASED (0x004C86B4)
#define ADDRESS_CONTROLLER_IS_KEY_REPEATED (0x004C863C)
#define ADDRESS_CONTROLLER_IS_KEY_DOWN (0x004C8604)
#define ADDRESS_CONTROLLER_GET_REPEATED_KEY (0x004C867C)
#define ADDRESS_DEVICE_GET_DPAD (0x004C80E0)
#define ADDRESS_DPAD_IS_PRESSED (0x004C8030)
#define ADDRESS_DPAD_IS_REPEATED (0x004C7F50)
#define ADDRESS_DPAD_IS_DOWN (0x004C7EF8)
#define ADDRESS_DPAD_IS_DOWN_2 (0x004C7BEC)
#define ADDRESS_DPAD_IS_RELEASED_2 (0x004C7C0C)
#define ADDRESS_DEVICE_GET_TOUCHSCREEN (0x004C810C)
#define ADDRESS_TOUCHSCREEN_IS_PRESSED (0x004C77D4)
#define ADDRESS_TOUCHSCREEN_IS_REPEATED (0x004C7774)
#define ADDRESS_TOUCHSCREEN_IS_RELEASED (0x004C77AC)
#define ADDRESS_TOUCHSCREEN_IS_DOWN (0x004C79F8)
#define ADDRESS_TOUCHSCREEN_GET_X (0x004C794C)
#define ADDRESS_TOUCHSCREEN_GET_Y (0x004C7974)
/** @} */
