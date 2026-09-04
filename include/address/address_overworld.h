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

/** * @name Overworld, Map & Weather Addresses
 * @{ */
#define ADDRESS_CHANGE_MAP (0x003D6258)
#define PROCESS_NAME_FIELD_MAP ("N5field12FieldmapProcE")
#define ADDRESS_MAP_BLOCK_VTABLE (0x007FD7DC)
#define ADDRESS_OVERWORLD_GET_MAP_TILE (0x00403DAC)
#define ADDRESS_OVERWORLD_GET_PLAYER_MOVEMENT (0x00782A8C)
#define ADDRESS_LOAD_MAP_DATA (0x003D9BD4)
#define ADDRESS_GET_NAVI_DEX_TABLE (0x004013A4)
#define ADDRESS_GET_ENCOUNTER_CONTACT_ACTION (0x0076E10C)
#define ADDRESS_GET_OVERWORLD_BACKGROUND_MUSIC (0x003C79F8)
#define ADDRESS_OVERWORLD_UPDATE_ZONE (0x0076D6F4)
#define ADDRESS_OVERWORLD_VTABLE (0x007FA2DC)
#define ADDRESS_UPDATE_ZONE_WEATHER (0x00491A5C)
#define ADDRESS_UPDATE_AREA_WEATHER (0x00491A74)
#define ADDRESS_OVERWORLD_CREATE_MODELS (0x003F8358)
/** @} */

/** * @name Overworld Code Patch Addresses
 * @{ */
#define ADDRESS_OVERWORLD_GET_DRAW_MODEL (0x004E3F90)
#define ADDRESS_OVERWORLD_FLY_CHECK (0x003EF8C4)
#define ADDRESS_OVERWORLD_SIMULATE_BUTTON_PRESS (0x00715C48)
/** @} */
