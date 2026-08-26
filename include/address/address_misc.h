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

/** * @name Miscellaneous Addresses
 * @{ */
#define ADDRESS_CORE (0x0063106C)
#define ADDRESS_UPDATE_FRAME (0x0011EEA4)
#define ADDRESS_EVENT_TABLE_CHECK_FLAG (0x0012FE90)
#define ADDRESS_EVENT_TABLE_RESET_FLAG (0x0011FB38)
#define ADDRESS_EVENT_TABLE_SET_FLAG (0x00406E94)
#define ADDRESS_ALLOC (0x001235A8)
#define ADDRESS_SCRIPT_FUNC_TABLE (0x007EA58C)
#define ADDRESS_DO_FIELD_MOVE (0x003D40EC)
#define ADDRESS_RECORD_MAX_VALUE_TABLE (0x0058DE14) // 999999999, 9999999, etc.
#define ADDRESS_RECORD_MAX_VALUE_INDEX_TABLE (0x0058DD4A) // 0, 0, 0, 1, 1, etc.
#define ADDRESS_IS_SHINY (0x00168F48)
#define ADDRESS_LOAD_SHOP_ITEMS (0x00346554)
#define ADDRESS_TRAINER_MODEL_TABLE (0x00586B8A)
#define ADDRESS_SCRIPT_INITIALIZE_VIRTUAL_MACHINE (0x00505DB4)
#define ADDRESS_SCRIPT_PAWN_BASE_LOAD (0x003AB0C8)
#define ADDRESS_CHECK_REGULATION (0x006F4F80) // app poke list static work
// 0x8072520, 0x807251C, 0x8072510
#define ADDRESS_LOAD_CRO_FILE (0x00110E2C)
#define ADDRESS_ON_SAVE_DATA_FILE (0x0036C47C)
#define ADDRESS_ON_UPDATE_FRAME (0x0011EEA4)
#define ADDRESS_KEYBOARD_UPDATE_KEYS (0x00744540)
/** @} */
