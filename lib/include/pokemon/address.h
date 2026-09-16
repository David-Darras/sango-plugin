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

namespace pokemon {
namespace address {

constexpr uptr kAppStatusSetupPokemonInfo = GAME_ADDRESS(0, 0x007144FC);
constexpr uptr kDataAccessorEncrypt = GAME_ADDRESS(0, 0x00168DCC);
constexpr uptr kDataAccessorDecrypt = GAME_ADDRESS(0, 0x00168E3C);
constexpr uptr kDataAccessorInitialize = GAME_ADDRESS(0, 0x0012C2BC);
constexpr uptr kUtilsLoadPokemonExperienceTable = GAME_ADDRESS(0, 0x0016886C);
constexpr uptr kUtilsGetPokemonMinimumExperience = GAME_ADDRESS(0, 0x00168A28);
constexpr uptr kUtilsGetPokemonLevelFromExperience =
    GAME_ADDRESS(0, 0x0014FDAC);
constexpr uptr kUtilsIsShiny = GAME_ADDRESS(0, 0x00168F48);
constexpr uptr kUtilsToShiny = GAME_ADDRESS(0, 0x00168F6C);
constexpr uptr kUtilsToNormal = GAME_ADDRESS(0, 0x00168FCC);
constexpr uptr kUtilsFromBallIdToItemId = GAME_ADDRESS(0, 0x003B7300);
constexpr uptr kGetSpeciesName = GAME_ADDRESS(0, 0x003B08F8);
constexpr uptr kGetDefaultNickname = GAME_ADDRESS(0, 0x00139E40);
constexpr uptr kSetAbilityName = GAME_ADDRESS(0, 0x0045108C);
constexpr uptr kSetMoveName = GAME_ADDRESS(0, 0x004509F4);
constexpr uptr kGetAbilityName = GAME_ADDRESS(0, 0x003B0D1C);
constexpr uptr kGetAbilityDescription = GAME_ADDRESS(0, 0x003B0CF0);
constexpr uptr kGetMoveName = GAME_ADDRESS(0, 0x003B5C54);
constexpr uptr kLoadMoveData = GAME_ADDRESS(0, 0x00175FB4);
constexpr uptr kItemDataGetName = GAME_ADDRESS(0, 0x003B7A64);
constexpr uptr kMoveDataTable = GAME_ADDRESS(0, 0x0062B3A0);
constexpr uptr kCreatePokemon = GAME_ADDRESS(0, 0x0046FE44);
constexpr uptr kAddPokemonToTeam = GAME_ADDRESS(0, 0x003B6754);
constexpr uptr kAppStatusUpdatePokemon = GAME_ADDRESS(0, 0x007122B4);
constexpr uptr kGetStats = GAME_ADDRESS(0, 0x004D30BC);
constexpr uptr kGetEvs = GAME_ADDRESS(0, 0x004D2A2C);
constexpr uptr kGetIvs = GAME_ADDRESS(0, 0x004D2AB4);
constexpr uptr kPokedexFormTable = GAME_ADDRESS(0, 0x0058C4A4);
constexpr uptr kPokedexGetFormIndex = GAME_ADDRESS(0, 0x004EB37C);
constexpr uptr kCheckEggHatching = GAME_ADDRESS(0, 0x00715D8C);
constexpr uptr kDayCareUpdate = GAME_ADDRESS(0, 0x007111F8);
constexpr uptr kEggHatch = GAME_ADDRESS(0, 0x00715EF0);
constexpr uptr kEggGeneration = GAME_ADDRESS(0, 0x00711364);
constexpr uptr kDayCareMaxExp = GAME_ADDRESS(0, 0x00465A34);
constexpr uptr kReplacePokemonModel = GAME_ADDRESS(0, 0x004713FC);
constexpr uptr kEncounterSetPokemon = GAME_ADDRESS(0, 0x0078DB30);
constexpr uptr kItemDataInitialize = GAME_ADDRESS(0, 0x003B7B9C);
constexpr uptr kItemDataGetParam = GAME_ADDRESS(0, 0x004D3C84);
constexpr uptr kLoadEvolutionTable = GAME_ADDRESS(0, 0x003B1108);
constexpr uptr kLoadMegaEvolutionTable = GAME_ADDRESS(0, 0x00168974);
constexpr uptr kScriptCreatePokemon = GAME_ADDRESS(0, 0x0077279C);
constexpr uptr kScriptAddPokemonToTeam = GAME_ADDRESS(0, 0x0073F474);
constexpr uptr kGiftPokemonTable = GAME_ADDRESS(0, 0x007EC06C);
constexpr uptr kTradePokemonTable = GAME_ADDRESS(0, 0x00586AE8);
constexpr uptr kStaticEncounterTable = GAME_ADDRESS(0, 0x007E4B20);
constexpr uptr kHealTeam = GAME_ADDRESS(0, 0x003B5FC0);
constexpr uptr kRemovePokemonFromTeam = GAME_ADDRESS(0, 0x003B6098);
constexpr uptr kResetNickname = GAME_ADDRESS(0, 0x003B51C0);
constexpr uptr kUpdateRuntimeData = GAME_ADDRESS(0, 0x003B360C);
constexpr uptr kInitializePokemon = GAME_ADDRESS(0, 0x0011F754);
constexpr uptr kBattleCheckPokemonCaptured = GAME_ADDRESS(0, 0x007247C8);
constexpr uptr kTechnicalMachineMoveTable = GAME_ADDRESS(0, 0x005A69E2);
constexpr uptr kDatabase = GAME_ADDRESS(0, 0x00617A00); // pml::Library
constexpr uptr kLoadMovepool = GAME_ADDRESS(0, 0x0014EB80);
constexpr uptr kMovepool = GAME_ADDRESS(0, 0x08029330);
constexpr uptr kItemEvTotalLimitCheck = GAME_ADDRESS(0, 0x0047888C);
constexpr uptr kItemEvTotalLimitCheck2 = GAME_ADDRESS(0, 0x004788F0);
constexpr uptr kItemEvTotalLimitClamp = GAME_ADDRESS(0, 0x004788FC);
constexpr uptr kItemEvStatLimitCheck1 = GAME_ADDRESS(0, 0x0046E104);
constexpr uptr kItemEvStatLimitCheck2 = GAME_ADDRESS(0, 0x0046E1A4);
constexpr uptr kItemEvStatLimitCheck3 = GAME_ADDRESS(0, 0x0046E244);
constexpr uptr kItemEvStatLimitCheck4 = GAME_ADDRESS(0, 0x0046E2E0);
constexpr uptr kItemEvStatLimitCheck5 = GAME_ADDRESS(0, 0x0046E37C);
constexpr uptr kItemEvStatLimitCheck6 = GAME_ADDRESS(0, 0x0046E41C);

} // namespace address
} // namespace pokemon
