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

namespace battle {
namespace address {

constexpr uptr kConfigSetupWild = GAME_ADDRESS(0x001CCC9C, 0x001D6CD8);
constexpr uptr kConfigSetupTrainer = GAME_ADDRESS(0x001CCDD0, 0x001D6E0C);

constexpr uptr kVtable = GAME_ADDRESS(0x00783DD0, 0x007D85D0);
constexpr uptr kMainProcess = GAME_ADDRESS(0, 0x08C69214);
constexpr uptr kLevelUp = GAME_ADDRESS(0x0070DEB0, 0x0075D73C);
constexpr uptr kStartMegaEvolutionAnimation = GAME_ADDRESS(0x006E465C, 0x0072EF18);
constexpr uptr kStartEntryAnimation = GAME_ADDRESS(0x007255BC, 0x00775354);
constexpr uptr kStartBackgroundMusic = GAME_ADDRESS(0x00425238, 0x0044D560);
constexpr uptr kPlayAnimation = GAME_ADDRESS(0x007027AC, 0x007510A8);
constexpr uptr kUpdateView = GAME_ADDRESS(0x006EE0AC, 0x0073B60C);
constexpr uptr kUpdateGauge = GAME_ADDRESS(0x006D4578, 0x0071E174);
constexpr uptr kTypeChart = GAME_ADDRESS(0x0077A2A8, 0x007CE428);
constexpr uptr kGetEffectiveness = GAME_ADDRESS(0x00719DB8, 0x00769D1C);
constexpr uptr kGetPokemonTypes = GAME_ADDRESS(0x00719E4C, 0x00769DB0);

constexpr uptr kSituationBegin = GAME_ADDRESS(0x0071503C, 0x007648D0);
constexpr uptr kSituationEnd = GAME_ADDRESS(0x00714D38, 0x007645CC);
constexpr uptr kSituationGet = GAME_ADDRESS(0x00714DD4, 0x00764668);
constexpr uptr kSituationSet = GAME_ADDRESS(0x007149D8, 0x0076426C);

constexpr uptr kControllerCreateMutation = GAME_ADDRESS(0x00713434, 0x00762CC0);
constexpr uptr kControllerApplyMutation = GAME_ADDRESS(0x00711000, 0x0076088C);
constexpr uptr kControllerSetMessage = GAME_ADDRESS(0x006F661C, 0x00744210);
constexpr uptr kControllerFillMessageSlot = GAME_ADDRESS(0x006F65E4, 0x007441D8);
constexpr uptr kControllerGetPokemon = GAME_ADDRESS(0x006FD7E8, 0x0074BC4C);
constexpr uptr kControllerExecuteMove = GAME_ADDRESS(0x006C4028, 0x0070EC48);
constexpr uptr kControllerSetWeather = GAME_ADDRESS(0x006F4004, 0x00741CC0);
constexpr uptr kBroadcasterRegister = GAME_ADDRESS(0x006F6308, 0x00743EFC);
constexpr uptr kBroadcasterFindListener = GAME_ADDRESS(0x006F6708, 0x007442FC);
constexpr uptr kBroadcasterUnregister = GAME_ADDRESS(0x006F6514, 0x00744108);
constexpr uptr kAddTerrain = GAME_ADDRESS(0x006CE120, 0x00718B10);

constexpr uptr kRegisterAbilityListener = GAME_ADDRESS(0x00710DB0, 0x0076063C);
constexpr uptr kRegisterMoveListener = GAME_ADDRESS(0x0071328C, 0x00762B18);

constexpr uptr kLoadAnimation = GAME_ADDRESS(0x006FF9EC, 0x0074E908);

constexpr uptr kGetHpGaugePane = GAME_ADDRESS(0x0048D760, 0x004BCD08);
constexpr uptr kPokemonModelTable = GAME_ADDRESS(0, 0x083F84C0);

constexpr uptr kMenuEntryHpPp = GAME_ADDRESS(0x00777084, 0x007CB09C);
constexpr uptr kMenuEntryBall = GAME_ADDRESS(0x0077709C, 0x007CB0B4);
constexpr uptr kMenuEntryStatus = GAME_ADDRESS(0x007770B4, 0x007CB0CC);
constexpr uptr kMenuEntryBattle = GAME_ADDRESS(0x007770CC, 0x007CB0E4);

constexpr uptr kMasterBallCheck = GAME_ADDRESS(0x006D8334, 0x007227A4);
constexpr uptr kMasterBallBranch = GAME_ADDRESS(0x006D8348, 0x007227B8);

constexpr uptr kBallCatchRate = GAME_ADDRESS(0x006D8EF4, 0x007232E4);

constexpr uptr kMegaRestrictionCheck = GAME_ADDRESS(0x006B63D4, 0x007007C0);
constexpr uptr kMegaRestrictionCheck2 = GAME_ADDRESS(0x006B378C, 0x006FDA74);
constexpr uptr kCanMegaEvolved = GAME_ADDRESS(0x004A2A28, 0x004D2970);

} // namespace address
} // namespace battle
