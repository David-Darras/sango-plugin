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

namespace overworld {
namespace address {

constexpr uptr kChangeMap = GAME_ADDRESS(0x003B9F14, 0x003D6258);
constexpr uptr kSetDefaultPosition = GAME_ADDRESS(0x003E2B00, 0x004040E4);
constexpr uptr kMapBlockVtable = GAME_ADDRESS(0x007B17F4, 0x007FD7DC); // XY: unverified (from neighbouring vtable)
constexpr uptr kGetMapTile = GAME_ADDRESS(0x003E28D8, 0x00403DAC);
constexpr uptr kGetPlayerMovement = GAME_ADDRESS(0x007338E4, 0x00782A8C);
constexpr uptr kLoadMapData = GAME_ADDRESS(0x003BCFA0, 0x003D9BD4);
constexpr uptr kGetNaviDexTable = GAME_ADDRESS(0, 0x004013A4);
constexpr uptr kGetEncounterContactAction = GAME_ADDRESS(0, 0x0076E10C);
constexpr uptr kGetOverworldBackgroundMusic = GAME_ADDRESS(0x003B0B10, 0x003C79F8);
constexpr uptr kUpdateZone = GAME_ADDRESS(0x0071DC34, 0x0076D6F4);
constexpr uptr kVtable = GAME_ADDRESS(0x007AE4BC, 0x007FA2DC);
constexpr uptr kCallScriptVtable = GAME_ADDRESS(0x0059C630, 0x005DF018); // XY: unverified (from neighbouring vtables)
constexpr uptr kUpdateZoneWeather = GAME_ADDRESS(0, 0x00491A5C);
constexpr uptr kUpdateAreaWeather = GAME_ADDRESS(0, 0x00491A74);
constexpr uptr kWeatherSetZone = GAME_ADDRESS(0x00462AA4, 0);
constexpr uptr kCreateModels = GAME_ADDRESS(0, 0x003F8358);
constexpr uptr kLoadWorldLayout = GAME_ADDRESS(0x003E4E2C, 0x004072A4);
constexpr uptr kMapSettingsTable = GAME_ADDRESS(0x005B1650, 0x005F45BC);
constexpr u32 kMapSettingsTableCount = 536;

constexpr uptr kLoadMapCharacters = GAME_ADDRESS(0x003BE640, 0x003DBB4C);
constexpr uptr kCompleteRegionModelList = GAME_ADDRESS(0x003D7654, 0x003F7640);
constexpr uptr kGetModelArchiveIndex = GAME_ADDRESS(0x0019B02C, 0x0019C678);

constexpr uptr kGetDrawModel = GAME_ADDRESS(0x004ACAC8, 0x004E3F90);
constexpr uptr kCallStaticEncounter = GAME_ADDRESS(0, 0x0077D0C4);
constexpr uptr kTradePokemon = GAME_ADDRESS(0x0073D180, 0x0074AC64);
constexpr uptr kFlyCheck = GAME_ADDRESS(0, 0x003EF8C4);
constexpr uptr kSimulateButtonPress = GAME_ADDRESS(0x006D7A74, 0x00715C48);

} // namespace address
} // namespace overworld
