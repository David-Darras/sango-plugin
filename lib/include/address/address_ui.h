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

/** * @name App Status & Layout Manager (UI) Addresses
 * @{ */
#define ADDRESS_APP_STATUS_GET_STAT_TRAMPOLINE (0x0070BA2C)
#define ADDRESS_LANGUAGE_ID (0x5F4F01)
#define PROCESS_NAME_APP_STATUS ("N3app6status7ProcessE")
#define ADDRESS_APP_STATUS_VTABLE (0x0071C3BC)
#define ADDRESS_APP_POKE_LIST_VTABLE (0x00705B08)
#define ADDRESS_KEYBOARD_VTABLE (0x0074B628)
#define ADDRESS_FONT_HAS_GLYPH (0x004C4B40)
#define ADDRESS_APP_LAYOUT_MANAGER_SET_TEXT_BOX_COLOR (0x00362828)
#define ADDRESS_APP_LAYOUT_MANAGER_SET_TEXT_BOX_INTEGER_VALUE (0x003629B4)
#define ADDRESS_APP_LAYOUT_MANAGER_SHOW_PANE (0x00361A10)
#define ADDRESS_APP_LAYOUT_MANAGER_SHOW_ANIMATION (0x361410)
#define ADDRESS_APP_LAYOUT_MANAGER_HIDE_ANIMATION (0x363334)
#define ADDRESS_APP_LAYOUT_MANAGER_GET_TOKEN (0x004C6594)
#define ADDRESS_APP_LAYOUT_MANAGER_SET_TEXT_BOX_STRING (0x4195F4)
#define ADDRESS_APP_LAYOUT_MANAGER_GET_TEXT_BOX (0x4C8CE0)
/** @} */

/** * @name Keyboard Addresses
 * @{ */
#define ADDRESS_KEYBOARD_WORD_FILTER (0x003A47C0)
#define ADDRESS_KEYBOARD_WORD_FILTER_RETURN (0x003A47C4)
#define ADDRESS_KEYBOARD_CALCULATE_KEY_INDEX (0x004C5924)
#define ADDRESS_KEYBOARD_REFRESH_ON_L (0x0074323C)
/// Base of the keyboard's key handler table (entries are 4 bytes wide).
#define ADDRESS_KEYBOARD_KEY_HANDLER_TABLE (0x00742F58)
#define ADDRESS_KEYBOARD_MODE_SWITCH (0x00746294)
#define ADDRESS_KEYBOARD_ANIMATION_1 (0x00743250)
#define ADDRESS_KEYBOARD_ANIMATION_2 (0x0074326C)
#define ADDRESS_KEYBOARD_ANIMATION_3 (0x007432DC)
#define ADDRESS_KEYBOARD_ANIMATION_4 (0x007432F8)
/** @} */

/** * @name Additional Layout Manager Accessors
 * @{ */
#define ADDRESS_APP_LAYOUT_MANAGER_GET_PICTURE (0x004C8C88)
#define ADDRESS_APP_LAYOUT_MANAGER_GET_PANE (0x004C9140)
/** @} */
