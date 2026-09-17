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

#include "core/game.h"
#include "core/types.h"

namespace ui {
namespace address {

constexpr uptr kAppStatusGetStatTrampoline = GAME_ADDRESS(0, 0x0070BA2C);
constexpr uptr kLanguageId = GAME_ADDRESS(0x005B1FE1, 0x005F4F01);
constexpr uptr kAppStatusVtable = GAME_ADDRESS(0, 0x0071C3BC);
constexpr uptr kAppPokeListVtable = GAME_ADDRESS(0, 0x00705B08);
constexpr uptr kKeyboardVtable = GAME_ADDRESS(0, 0x0074B628);
constexpr uptr kFontHasGlyph = GAME_ADDRESS(0x00494F64, 0x004C4B40);
constexpr uptr kAppLayoutManagerSetTextBoxColor = GAME_ADDRESS(0x0034A334, 0x00362828);
constexpr uptr kAppLayoutManagerSetTextBoxIntegerValue = GAME_ADDRESS(0x0034A4C0, 0x003629B4);
constexpr uptr kAppLayoutManagerShowPane = GAME_ADDRESS(0x00349488, 0x00361A10);
constexpr uptr kAppLayoutManagerShowAnimation = GAME_ADDRESS(0x00348F78, 0x00361410);
constexpr uptr kAppLayoutManagerHideAnimation = GAME_ADDRESS(0x0034ADE0, 0x00363334);
constexpr uptr kAppLayoutManagerGetToken = GAME_ADDRESS(0x00496BD0, 0x004C6594);
constexpr uptr kAppLayoutManagerSetTextBoxString = GAME_ADDRESS(0x003F6BC4, 0x004195F4);
constexpr uptr kAppLayoutManagerGetTextBox = GAME_ADDRESS(0x0049928C, 0x004C8CE0);

constexpr uptr kKeyboardWordFilter = GAME_ADDRESS(0x0038DE50, 0x003A47C0);
constexpr uptr kKeyboardWordFilterReturn = GAME_ADDRESS(0x0038DE54, 0x003A47C4);
constexpr uptr kKeyboardCalculateKeyIndex = GAME_ADDRESS(0x00495CA0, 0x004C5924);
constexpr uptr kKeyboardRefreshOnL = GAME_ADDRESS(0, 0x0074323C);

constexpr uptr kKeyboardKeyHandlerTable = GAME_ADDRESS(0, 0x00742F58);
constexpr uptr kKeyboardModeSwitch = GAME_ADDRESS(0, 0x00746294);
constexpr uptr kKeyboardAnimation1 = GAME_ADDRESS(0, 0x00743250);
constexpr uptr kKeyboardAnimation2 = GAME_ADDRESS(0, 0x0074326C);
constexpr uptr kKeyboardAnimation3 = GAME_ADDRESS(0, 0x007432DC);
constexpr uptr kKeyboardAnimation4 = GAME_ADDRESS(0, 0x007432F8);

constexpr uptr kAppLayoutManagerGetPicture = GAME_ADDRESS(0x00499234, 0x004C8C88);
constexpr uptr kAppLayoutManagerGetPane = GAME_ADDRESS(0x004996EC, 0x004C9140);

} // namespace address
} // namespace ui
