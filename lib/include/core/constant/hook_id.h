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
 * @file hook_id.h
 * @brief The ids of all the hooks.
 *
 * Each hook has one id. The library uses the ids before kProduct0.
 * A product uses kProduct0 to kProduct15.
 */

#pragma once

#include <types.h>

namespace core {

/// The id of a hook. See core::HookManager.
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
  kPokemonModelSettings,
  kCheckAppRequest,
  kAppStatusSetupGraphicsParams,
  kAppStatusSetupGraphicsMoves,
  kAppStatusSetupGraphicsContest,
  kAppStatusSetupGraphicsInfos,
  kLoadEvolutionTable,
  kItemDataGetParam,
  kAddPokemonToTeam,
  kGetAbilityName,
  kMessageGetString,
  kBattleRegisterAbilityListener,
  kBattleRegisterMoveListener,
  kBattleLoadAnimation,
  kBattleLoadEffect,
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
  kGetDexNavTable,
  kLoadMapData,
  kBattleStartMegaEvolutionAnimation,
  kBattleStartEntryAnimation,
  kBattleStartBackgroundMusic,
  kBattlePlayAnimation,
  kGetOverworldBackgroundMusic,
  kReplacePokemonModel,
  kReadFileAsync,
  kReadFileAsync2,
  kArchiveLoadData,
  kArchiveLoadCompressedFile,
  kArchiveGetFileSize,
  kArchiveGetFileSize2,
  kArchiveGetInfo,
  kArchiveRead,
  kArchiveLoadData2,
  kMainProcessLoop,
  kMainEventLoop,
  kLoadMegaEvolutionTable,
  kGetMegaEvolvedFormNo,
  kGetSpeciesName,
  kLoadMovepool,
  kParticleCreate,
  kResourceAttachBufferAndSetup,
  kScriptAddPokemonToTeam,
  kCallStaticEncounter,
  kTradePokemon,
  kLoadMapCharacters,
  kChangeMap,
  kCompleteRegionModelList,
  kLoadWorldLayout,
  kScriptDescriptorSetup,
  kUpdateZoneWeather,
  kUpdateAreaWeather,
  kOverworldSetDefaultPosition,
  kModelPlayAnimation,
  kModelUpdateMotion,
  kTitleSequenceSync,
  kCallPokemonList,
  kGetSystemDateTime,
  kPlayerCheckPushEvent,

  /// Free ids for the hooks of a product (kaizo, undertow, your ROM hack).
  /// The library never uses them. See docs/tutorials/03-add-a-hook.md.
  kProduct0,
  kProduct1,
  kProduct2,
  kProduct3,
  kProduct4,
  kProduct5,
  kProduct6,
  kProduct7,
  kProduct8,
  kProduct9,
  kProduct10,
  kProduct11,
  kProduct12,
  kProduct13,
  kProduct14,
  kProduct15,

  kMax ///< The number of hook ids.
};

} // namespace core

using core::HookId;
