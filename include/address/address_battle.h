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

/** * @name Battle Addresses
 * @{ */
#define ADDRESS_BATTLE_MAIN_PROCESS (0x08C69214)
#define ADDRESS_BATTLE_SETUP_CONFIG (0x001D6CD8)
#define ADDRESS_SETUP_TRAINER_BATTLE_CONFIG (0x001D6E0C)
#define PROCESS_NAME_BATTLE ("N3btl14BattleMainProcE")
#define ADDRESS_BATTLE_UPDATE_EXP (0x0075D73C)
#define ADDRESS_BATTLE_START_MEGA_EVOLVE_ANIMATION (0x0072EF18)
#define ADDRESS_BATTLE_START_BATTLE_ANIMATION (0x00775354)
#define ADDRESS_BATTLE_START_BACKGROUND_MUSIC (0x0044D560)
#define ADDRESS_BATTLE_PLAY_ANIMATION (0x007510A8)
#define ADDRESS_UPDATE_BATTLE_VIEW (0x0073B60C)
#define ADDRESS_BATTLE_VTABLE (0x007D85D0)
#define ADDRESS_BATTLE_UPDATE_GAUGE (0x0071E174)
#define ADDRESS_BATTLE_UPDATE_VIEW (0x0073B60C)
/** @} */

/** * @name Battle Code Patch & Handler Addresses
 * @{ */
#define ADDRESS_BATTLE_GET_VARIABLE (0x00764668)
#define ADDRESS_BATTLE_UPDATE_WEATHER (0x00741CC0)
#define ADDRESS_BATTLE_GET_EVENT_HANDLER (0x00743EFC)
#define ADDRESS_BATTLE_GET_ABILITY_HANDLER (0x0076063C)
#define ADDRESS_BATTLE_GET_MOVE_HANDLER (0x00762B18)
#define ADDRESS_BATTLE_LOAD_ANIMATION (0x0074E908)
#define ADDRESS_BATTLE_HP_GAUGE_GET_PANE (0x004BCD08)
#define ADDRESS_BATTLE_POKEMON_MODEL_TABLE (0x083F84C0)
#define ADDRESS_BATTLE_MENU_ENTRY_HP_PP (0x007CB09C)
#define ADDRESS_BATTLE_MENU_ENTRY_BALL (0x007CB0B4)
#define ADDRESS_BATTLE_MENU_ENTRY_STATUS (0x007CB0CC)
#define ADDRESS_BATTLE_MENU_ENTRY_BATTLE (0x007CB0E4)
#define ADDRESS_BATTLE_MASTER_BALL_CHECK (0x007227A4)
#define ADDRESS_BATTLE_MASTER_BALL_BRANCH (0x007227B8)
#define ADDRESS_BATTLE_BALL_CATCH_RATE (0x007232E4)
#define ADDRESS_BATTLE_BALL_CATCH_RATE_RETURN (0x007232E8)
#define ADDRESS_BATTLE_MEGA_RESTRICTION_CHECK (0x007007C0)
#define ADDRESS_BATTLE_MEGA_RESTRICTION_CHECK_2 (0x006FDA74)
/** @} */
