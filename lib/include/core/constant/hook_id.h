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

#include <types.h>

namespace core {

/**
 * @brief Identifiers for specific input-related function hooks.
 * Used by the HookManager to track and manage multiple hooks.
 */
enum class HookId : u32 {
  kEntrypoint,
  kIsKeyPressed,
  kIsKeyReleased,
  kIsKeyDown,
  kIsKeyRepeated,
  kIsDPadDown,
  kIsDPadRepeated,
  kIsTouchDown,
  kIsTouchReleased,
  kGetRepeatedKey,
  kUpdateMatrices,
  kUpdateLookAt,
  kGetPlayerMovement,
  kBattleCheckPokemonCaptured,
  kBattleUpdateGauge,
  kBattleUpdateView,
  kBattleConfigSetupWild,
  kBattleConfigSetupTrainer,
  kChangeOutlineScale,
  kChangeAmbientLightColor,
  kChangeDiffuseLightColor,
  kDrawPicture,
  kDrawTextBox,
  kGetMapTile,
  kUpdateFrame,
  kStartBackupThread,
  kSceneRegister0,
  kCallApp,
  kLoadShopItems,
  kUnloadShopItems,
  kShopGetItemName,
  kShopGetItemDescription,
  kShopDisplayItemDescription,
  kShopPurchaseItem,
  kBagAddItem,
  kBattleLevelUp,
  kCheckAppRequest,
  kAppStatusSetupGraphicsParams,
  kAppStatusSetupGraphicsMoves,
  kAppStatusSetupGraphicsContest,
  kAppStatusSetupGraphicsInfos,
  kLoadEvolveTable,
  kGlobalDataItemGetParam,
  kAddPokemonToTeam,
  kGetAbilityName,
  kMessageGetString,
  kBattleRegisterAbilityListener,
  kBattleRegisterMoveListener,
  kBattleLoadAnimation,
  kBattleAddTerrain,
  kSetAbilityName,
  kSetMoveName,
  kGetAbilityDescription,
  kLoadCro,
  kGameTextManagerGetText,
  kGetMoveName,
  kLoadMoveData,
  kKeyboardUpdateKeys,
  kInitializePokemon,
  kIsShiny,
  kFromNormalToShiny,
  kFromShinyToNormal,
  kLoadScript,
  kGetEncounterPokemon,
  kGetNaviDexTable,
  kLoadMapData,
  kBattleStartMegaEvolutionAnimation,
  kBattleStartEntryAnimation,
  kBattleStartBackgroundMusic,
  kBattlePlayAnimation,
  kGetOverworldBackgroundMusic,
  kOverworldUpdateZone,
  kReplacePokemonModel,
  kReadFileAsync,
  kReadFileAsync2,
  kArchiveLoadData,
  kMainProcessLoop,
  kMainEventLoop,
  kGlobalDataLoadMegaEvolutionTable,
  kParticleCreate,
  kResourceAttachBufferAndSetup,
  kScriptAddPokemonToTeam,
  kCallStaticEncounter,
  kTradePokemon,
  kLoadMapCharacters,
  kCompleteRegionModelList,
  kLoadWorldLayout,
  kScriptDescriptorSetup,
  kUpdateZoneWeather,
  kUpdateAreaWeather,
  kOverworldSetDefaultPosition,
  kModelPlayAnimation,
  kModelUpdateMotion,
  kTitleSequenceSync,
  kMax
};

} // namespace core

using core::HookId;
