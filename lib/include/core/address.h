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
#include "system/address.h"

namespace core {
namespace address {

// Processes and title screen
constexpr uptr kTitle = GAME_ADDRESS(0, 0x00740184);
constexpr uptr kIntroVtable = GAME_ADDRESS(0x006B49F8, 0x007203C8);
constexpr uptr kIntroFsm = GAME_ADDRESS(0, 0x0071DAE8);
constexpr uptr kCinematicVtable = GAME_ADDRESS(0x006D318C, 0x0072518C);
constexpr uptr kTitleScreenVtable = GAME_ADDRESS(0x006C7354, 0x00747320);
constexpr uptr kIntroductionVtable = GAME_ADDRESS(0x006B49F8, 0x007203C8);
constexpr uptr kSelectStarterVtable = GAME_ADDRESS(0x006AF8C4, 0x006FA8DC);
constexpr uptr kEventScriptVtable = GAME_ADDRESS(0x0059C630, 0x005DF018); // XY: unverified (from neighbouring vtables)
constexpr uptr kStarter = GAME_ADDRESS(0, 0x0804F3F0);
constexpr uptr kStarterModel = GAME_ADDRESS(0, 0x08249898);

constexpr uptr kTitleScreenTopVideoId = GAME_ADDRESS(0, 0x00740378);
constexpr uptr kTitleScreenBottomVideoId = GAME_ADDRESS(0, 0x0074039C);
constexpr uptr kTitleScreenDelay = GAME_ADDRESS(0, 0x00740390);
constexpr uptr kTitleScreenShadow = GAME_ADDRESS(0, 0x00740104);
constexpr uptr kTitleScreenSequenceSync = GAME_ADDRESS(0, 0x00739778);
constexpr uptr kTitleScreenPokemonCrySpecies = GAME_ADDRESS(0, 0x00740498);
constexpr uptr kTitleScreenPokemonCryVolume = GAME_ADDRESS(0, 0x0074049C);

// Engine, scripts, shops...
constexpr uptr kCore = GAME_ADDRESS(0x005EF084, 0x0063106C);
constexpr uptr kUpdateFrame = GAME_ADDRESS(0x0011E9D0, 0x0011EEA4);
constexpr uptr kEventTableCheckFlag = GAME_ADDRESS(0x00140474, 0x0012FE90);
constexpr uptr kEventTableResetFlag = GAME_ADDRESS(0x0011F370, 0x0011FB38);
constexpr uptr kEventTableSetFlag = GAME_ADDRESS(0x003E4A1C, 0x00406E94);
constexpr uptr kAlloc = GAME_ADDRESS(0x00122C60, 0x001235A8);
constexpr uptr kDoFieldMove = GAME_ADDRESS(0x003B7A8C, 0x003D40EC);
constexpr uptr kRecordMaxValueTable = GAME_ADDRESS(0x0054D290, 0x0058DE14); // 999999999, 9999999, etc.
constexpr uptr kRecordMaxValueIndexTable = GAME_ADDRESS(0x0054D1C6, 0x0058DD4A); // 0, 0, 0, 1, 1, etc.
constexpr uptr kIsShiny = GAME_ADDRESS(0x00168AC8, 0x00168F48);
constexpr uptr kTrainerModelTable = GAME_ADDRESS(0x005481C0, 0x00586B8A); // XY: unverified (content match)
constexpr uptr kCheckRegulation =
    GAME_ADDRESS(0x006AAF70, 0x006F4F80); // app poke list static work
// 0x8072520, 0x807251C, 0x8072510
constexpr uptr kLoadCroFile = GAME_ADDRESS(0x00110EAC, 0x00110E2C);
constexpr uptr kStartBackupThread = GAME_ADDRESS(0x00431A68, 0x0045D6BC);
constexpr uptr kOnUpdateFrame = GAME_ADDRESS(0x0011E9D0, 0x0011EEA4);
constexpr uptr kKeyboardUpdateKeys = GAME_ADDRESS(sys::address::kMemoryRegionKeyboard + 0x22E8, 0x00744540);

constexpr uptr kScriptEngineInstance = GAME_ADDRESS(0x005B1748, 0x005F46FC);
constexpr uptr kScriptDescriptorSetup = GAME_ADDRESS(0x003DB430, 0x003FB594);
constexpr uptr kScriptPawnBaseLoad = GAME_ADDRESS(0x00394924, 0x003AB0C8);
constexpr uptr kScriptInitializeVirtualMachine = GAME_ADDRESS(0x004D01E4, 0x00505DB4);
constexpr uptr kScriptRaiseError = GAME_ADDRESS(0x004D073C, 0x0050630C);
constexpr uptr kScriptNativesField = GAME_ADDRESS(0x0079F2A0, 0x007EA58C);
constexpr uptr kScriptNativesState = GAME_ADDRESS(0x0079FC9C, 0x007EB234);
constexpr uptr kScriptNativesInteractive = GAME_ADDRESS(0x007A0304, 0x007EB99C);
constexpr uptr kScriptNativesPokemonCenter = GAME_ADDRESS(0x007A0364, 0x007EB9FC);
constexpr uptr kScriptNativesMapEffects = GAME_ADDRESS(0x007A0774, 0x007EBAAC);
constexpr uptr kScriptNativesBattleFacility = GAME_ADDRESS(0x007A0D64, 0x007EBB44);
constexpr uptr kScriptNativesFieldServices = GAME_ADDRESS(0x007A0EFC, 0x007EBCDC);
constexpr uptr kScriptNativesNpcAi = GAME_ADDRESS(0x007A0FAC, 0x007EBFBC);
constexpr uptr kScriptNativesProgram = GAME_ADDRESS(0x0054929C, 0x005885E8);
// The four places a field script waits for a key press, and the cutscene
// player - each returns "done" once patched.
constexpr uptr kScriptWaitKeyPress0 = GAME_ADDRESS(0x0070189C, 0x0073E86C);
constexpr uptr kScriptWaitKeyPress1 = GAME_ADDRESS(0x007076DC, 0x007449E0);
constexpr uptr kScriptWaitKeyPress2 = GAME_ADDRESS(0x003F6F0C, 0x00419984);
constexpr uptr kScriptWaitKeyPress3 = GAME_ADDRESS(0x003F6E28, 0x00419898);
constexpr uptr kScriptPlayCutscene = GAME_ADDRESS(0x0073E3C8, 0x0074BE5C);

constexpr uptr kLoadShopItems = GAME_ADDRESS(0, 0x00346554); // XY: near 0x003310xx (ItemContainer::Initialize)
constexpr uptr kUnloadShopItems = GAME_ADDRESS(0x003313A4, 0x00346AE0);
constexpr uptr kShopGetItemName = GAME_ADDRESS(0x00331348, 0x00346A44);
constexpr uptr kShopGetItemInfo = GAME_ADDRESS(0x00331324, 0x003469E0);
constexpr uptr kShopDisplayItemInfo = GAME_ADDRESS(0x003C68AC, 0x003E6550);
constexpr uptr kShopPurchaseItem = GAME_ADDRESS(0, 0x003E6764); // XY candidate 0x003C69BC (unverified)
constexpr uptr kBagAddItem = GAME_ADDRESS(0x004362FC, 0x00463750);

constexpr uptr kOverworldMenuAppAllocSize = GAME_ADDRESS(0x003BA6F4, 0x003D6D18);
constexpr uptr kTopMenuAppAllocSize = GAME_ADDRESS(0x003BB9C4, 0x003D81D4); // XY: unverified (patch site, 31/40 words)
constexpr uptr kTownMapCallback = GAME_ADDRESS(0, 0x003EF898);

} // namespace address
} // namespace core
