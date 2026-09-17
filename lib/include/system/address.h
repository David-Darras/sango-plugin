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

namespace sys {
namespace address {

// Filesystem
constexpr uptr kFileOpen = GAME_ADDRESS(0, 0x00123FA8);
constexpr uptr kFileRead = GAME_ADDRESS(0, 0x00145CF0);
constexpr uptr kFileWrite = GAME_ADDRESS(0, 0x001F5BE0);
constexpr uptr kFsMountSdmc = GAME_ADDRESS(0, 0x001F7CFC);
constexpr uptr kFsCreateFile = GAME_ADDRESS(0, 0x001F5C50);
constexpr uptr kFsDeleteFile = GAME_ADDRESS(0, 0x001F5CCC);
constexpr uptr kFsCreateDirectory = GAME_ADDRESS(0, 0x001F5F0C);
constexpr uptr kArchiveFilenameTable = GAME_ADDRESS(0, 0x005F5050);
constexpr uptr kArchiveReadFileAsync = GAME_ADDRESS(0, 0x0036DC68);
constexpr uptr kArchiveReadFileAsync2 = GAME_ADDRESS(0, 0x0036ED10);
constexpr uptr kArchiveLoadData = GAME_ADDRESS(0, 0x0036F030);
constexpr uptr kArchiveLoadData2 = GAME_ADDRESS(0, 0x00128E4C);
constexpr uptr kArchiveInitialize = GAME_ADDRESS(0, 0x0011CA2C);
constexpr uptr kArchiveGetFileSize = GAME_ADDRESS(0, 0x0011CA10);
constexpr uptr kArchiveLoadFile = GAME_ADDRESS(0, 0x00128B34);
constexpr uptr kArchiveLoadCompressedFile = GAME_ADDRESS(0, 0x00137A30);

// Input
constexpr uptr kDeviceGetController = GAME_ADDRESS(0x001174D0, 0x00117550);
constexpr uptr kControllerIsKeyPressed = GAME_ADDRESS(0x00498BBC, 0x004C86EC);
constexpr uptr kControllerIsKeyReleased = GAME_ADDRESS(0x00498B84, 0x004C86B4);
constexpr uptr kControllerIsKeyRepeated = GAME_ADDRESS(0x00498B0C, 0x004C863C);
constexpr uptr kControllerIsKeyDown = GAME_ADDRESS(0x00498AD4, 0x004C8604);
constexpr uptr kControllerGetRepeatedKey = GAME_ADDRESS(0x00498B4C, 0x004C867C);
constexpr uptr kDeviceGetDpad = GAME_ADDRESS(0x00498590, 0x004C80E0);
constexpr uptr kDpadIsPressed = GAME_ADDRESS(0x00498368, 0x004C8030);
constexpr uptr kDpadIsRepeated = GAME_ADDRESS(0x00498288, 0x004C7F50);
constexpr uptr kDpadIsDown = GAME_ADDRESS(0x00498230, 0x004C7EF8);
constexpr uptr kDpadIsDown2 = GAME_ADDRESS(0x00497F34, 0x004C7BEC);
constexpr uptr kDpadIsReleased2 = GAME_ADDRESS(0x00497F54, 0x004C7C0C);
constexpr uptr kDeviceGetTouchscreen = GAME_ADDRESS(0x004985CC, 0x004C810C);
constexpr uptr kTouchscreenIsPressed = GAME_ADDRESS(0x00497AA8, 0x004C77D4);
constexpr uptr kTouchscreenIsRepeated = GAME_ADDRESS(0x00497A48, 0x004C7774);
constexpr uptr kTouchscreenIsReleased = GAME_ADDRESS(0x00497A80, 0x004C77AC);
constexpr uptr kTouchscreenIsDown = GAME_ADDRESS(0x00497CCC, 0x004C79F8);
constexpr uptr kTouchscreenGetX = GAME_ADDRESS(0x00497C20, 0x004C794C);
constexpr uptr kTouchscreenGetY = GAME_ADDRESS(0x00497C48, 0x004C7974);

// Sound
constexpr uptr kSoundPlayPokemonCry = GAME_ADDRESS(0x00427988, 0x004500C0);
constexpr uptr kSoundChangePokemonCryVolume = GAME_ADDRESS(0x004265B4, 0x0044EB4C);
constexpr uptr kSoundPlaySoundEffect = GAME_ADDRESS(0x00427550, 0x0044FC88);
constexpr uptr kSoundPlayBackgroundMusic = GAME_ADDRESS(0x004260FC, 0x0044E6CC);

// System
constexpr uptr kMessageGetString = GAME_ADDRESS(0, 0x00139A34);
constexpr uptr kStringVtable = GAME_ADDRESS(0x0059B9F8, 0x005DE3BC);
constexpr uptr kEntrypoint =
    GAME_ADDRESS(0x001220D0, 0x00122938); // Render Home Button Blocked
constexpr uptr kGameManager = GAME_ADDRESS(0x005EDA08, 0x0062F7C4);
constexpr uptr kCallApp = GAME_ADDRESS(0, 0x003CFBA8);
constexpr uptr kOsReadOnlyKernelInfo = GAME_ADDRESS(0x1FF80000, 0x1FF80000);
constexpr uptr kHeapTable = GAME_ADDRESS(0, 0x08000000);
constexpr uptr kStdWcslen = GAME_ADDRESS(0, 0x001003E8 | 1);
constexpr uptr kStdVswprintf = GAME_ADDRESS(0x001004B4 | 1  , 0x001004B8 | 1);
constexpr u32 kBufferSize = 128;
constexpr uptr kGetRandomValue = GAME_ADDRESS(0, 0x0048AF80);
constexpr uptr kGetElapsedTime = GAME_ADDRESS(0, 0x0012B724);
constexpr uptr kConvertTimeToSeconds = GAME_ADDRESS(0, 0x0012FBDC);
constexpr uptr kDateTime = GAME_ADDRESS(0, 0x006174F8);
constexpr uptr kCheckAppRequest = GAME_ADDRESS(0, 0x007BDE50);
constexpr uptr kMainProcessLoop = GAME_ADDRESS(0x00394E88, 0x003AB62C);
constexpr uptr kMainEventLoop = GAME_ADDRESS(0x00117D7C, 0x00117E4C);
constexpr uptr kGameTextManagerGetText = GAME_ADDRESS(0, 0x00139A34);

constexpr uptr kHeapAlloc = GAME_ADDRESS(0, 0x0011ED58);
constexpr uptr kHeapFree = GAME_ADDRESS(0, 0x00139DA4);

constexpr uptr kProcessMemoryStart = GAME_ADDRESS(0, 0x00100000);
constexpr uptr kProcessMemoryEnd = GAME_ADDRESS(0, 0x00900000);

constexpr uptr kMemoryRegionGameCode = GAME_ADDRESS(0, 0x006F3000);
constexpr uptr kMemoryRegionAppStatus = GAME_ADDRESS(0, 0x0070B000);
constexpr uptr kMemoryRegionTitleScreen = GAME_ADDRESS(0, 0x00728000);
constexpr uptr kMemoryRegionKeyboard = GAME_ADDRESS(0, 0x00742000);

} // namespace address
} // namespace sys
