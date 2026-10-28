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
 * @file address.h
 * @brief The addresses of the overworld domain. Most of them are in the overworld CRO.
 */

#pragma once

#include "core/game.h"
#include "core/types.h"

namespace overworld {
namespace address {
constexpr uptr kChangeMap = GAME_ADDRESS(0x003B9F14, 0x003D6258);
constexpr uptr kSetDefaultPosition = GAME_ADDRESS(0x003E2B00, 0x004040E4);
constexpr uptr kMapBlockVtable = GAME_ADDRESS(0x007B17F4, 0x007FD7DC);
// XY: not verified (found near another vtable).
constexpr uptr kGetMapTile = GAME_ADDRESS(0x003E28D8, 0x00403DAC);
/// The table of the flags of the tiles: 256 numbers of 16 bits, indexed by the
/// ground number (the top byte of the attribute of a tile). The game gives a
/// tile the flags `(attribute & ~7) | table[ground]`: the bits 0 to 2 (wall,
/// water, wild Pokemon) come only from the table. ORAS only.
constexpr uptr kTileFlagTable = GAME_ADDRESS(0, 0x005860E4);
constexpr uptr kGetPlayerMovement = GAME_ADDRESS(0x007338E4, 0x00782A8C);
/// Checks if the player can walk to the next tile with the D-pad (ORAS only).
/// Returns the hit bits: the map limit, the tile attribute, an object, the
/// height, or the end of the map.
constexpr uptr kPlayerHitCheckGrid = GAME_ADDRESS(0, 0x00757698);
/// The same check for the walk with the Circle Pad (ORAS only).
constexpr uptr kPlayerHitCheckAnalog = GAME_ADDRESS(0, 0x00755C58);
/// Starts the command of an action of the player (a walk, a run...), by its
/// number (ORAS only). Parameters: the player, the command, the direction.
constexpr uptr kPlayerSetCommand = GAME_ADDRESS(0, 0x0076A6F4);
constexpr uptr kLoadMapData = GAME_ADDRESS(0x003BCFA0, 0x003D9BD4);
constexpr uptr kGetDexNavTable = GAME_ADDRESS(0, 0x004013A4);
constexpr uptr kGetEncounterContactAction = GAME_ADDRESS(0, 0x0076E10C);
constexpr uptr kGetOverworldBackgroundMusic = GAME_ADDRESS(
    0x003B0B10, 0x003C79F8);
constexpr uptr kUpdateZone = GAME_ADDRESS(0x0071DC34, 0x0076D6F4);
constexpr uptr kVtable = GAME_ADDRESS(0x007AE4BC, 0x007FA2DC);
constexpr uptr kCallScriptVtable = GAME_ADDRESS(0x0059C630, 0x005DF018);
// XY: not verified (found near other vtables).
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
/// Returns the chance (in percent) of a wild encounter at each step. ORAS only.
constexpr uptr kGetEncounterChance = GAME_ADDRESS(0, 0x006F7620);
constexpr uptr kCallStaticEncounter = GAME_ADDRESS(0, 0x0077D0C4);
/// Starts a trainer battle (game manager, parameter). The parameter has 16
/// bytes: see page_battle.cc. ORAS only.
constexpr uptr kStartTrainerBattle = GAME_ADDRESS(0, 0x003DA09C);
constexpr uptr kTradePokemon = GAME_ADDRESS(0x0073D180, 0x0074AC64);
constexpr uptr kFlyCheck = GAME_ADDRESS(0, 0x003EF8C4);
constexpr uptr kSimulateButtonPress = GAME_ADDRESS(0x006D7A74, 0x00715C48);

constexpr uptr kPlayerCheckPushEvent = GAME_ADDRESS(0, 0x0070B80C);
constexpr uptr kPlayerCallSwimOnEvent = GAME_ADDRESS(0, 0x0070B9E0);
constexpr uptr kPlayerIsSwimOn = GAME_ADDRESS(0, 0x00783008);
constexpr uptr kMoveModelGetFacing = GAME_ADDRESS(0, 0x006F3C30);
constexpr uptr kPartyGetMember = GAME_ADDRESS(0, 0x006F3B30);

constexpr uptr kBerryTreeLocationTable = GAME_ADDRESS(0, 0x007E4458);
constexpr uptr kBerryIdTable = GAME_ADDRESS(0, 0x005A695C);

constexpr uptr kHiddenItemPointer = GAME_ADDRESS(0, 0x001135E0);
constexpr uptr kRandomHiddenItemPointer = GAME_ADDRESS(0, 0x003E85E4);

constexpr uptr kDecorationTable = GAME_ADDRESS(0, 0x005837DC);
constexpr u32 kDecorationCount = 174;
constexpr u32 kDecorationEntrySize = 40;
} // namespace address
} // namespace overworld