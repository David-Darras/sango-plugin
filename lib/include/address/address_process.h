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

/** * @name Process & Screen VTable Addresses
 * @{ */
#define ADDRESS_TITLE (0x00740184)
#define ADDRESS_INTRO_VTABLE (0x007203C8)
#define ADDRESS_INTRO_FSM (0x0071DAE8)
#define ADDRESS_CINEMATIC_VTABLE (0x0072518C)
#define ADDRESS_TITLE_SCREEN_VTABLE (0x00747320)
#define ADDRESS_INTRODUCTION_VTABLE (0x007203C8)
#define ADDRESS_SELECT_STARTER_VTABLE (0x006FA8DC)
#define ADDRESS_EVENT_SCRIPT_VTABLE (0x005DF018)
#define ADDRESS_STARTER (0x0804F3F0)
/** @} */

/** * @name Title Screen Patch Addresses
 * @{ */
#define ADDRESS_TITLE_SCREEN_TOP_VIDEO_ID (0x00740378)
#define ADDRESS_TITLE_SCREEN_BOTTOM_VIDEO_ID (0x0074039C)
#define ADDRESS_TITLE_SCREEN_DELAY (0x00740390)
#define ADDRESS_TITLE_SCREEN_SHADOW (0x00740104)
#define ADDRESS_TITLE_SCREEN_POKEMON_CRY_SPECIES (0x00740498)
#define ADDRESS_TITLE_SCREEN_POKEMON_CRY_VOLUME (0x0074049C)
/** @} */
