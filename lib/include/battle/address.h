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
 * @brief The addresses of the battle domain. Most of them are in the battle CRO.
 */

#pragma once

#include "core/game.h"
#include "core/types.h"

namespace battle {
namespace address {

constexpr uptr kConfigSetupWild = GAME_ADDRESS(0x001CCC9C, 0x001D6CD8);
constexpr uptr kConfigSetupTrainer = GAME_ADDRESS(0x001CCDD0, 0x001D6E0C);

constexpr uptr kVtable = GAME_ADDRESS(0x00783DD0, 0x007D85D0);
/// The model of the trainer class: a number of TrainerModelId, or -1 for no
/// 3D model. It is in the main code, not in the battle CRO.
constexpr uptr kGetTrainerClassModel = GAME_ADDRESS(0, 0x004E7BD4);
/// The kind of entrance of the model of the trainer class: -1 for no model.
constexpr uptr kGetTrainerClassModelEntrance = GAME_ADDRESS(0, 0x004E7C14);
constexpr uptr kMainProcess = GAME_ADDRESS(0, 0x08C69214);
constexpr uptr kLevelUp = GAME_ADDRESS(0x0070DEB0, 0x0075D73C);
constexpr uptr kStartMegaEvolutionAnimation = GAME_ADDRESS(0x006E465C, 0x0072EF18);
constexpr uptr kStartEntryAnimation = GAME_ADDRESS(0x007255BC, 0x00775354);
constexpr uptr kPlayAnimation = GAME_ADDRESS(0x007027AC, 0x007510A8);
constexpr uptr kUpdateView = GAME_ADDRESS(0x006EE0AC, 0x0073B60C);
constexpr uptr kUpdateGauge = GAME_ADDRESS(0x006D4578, 0x0071E174);
constexpr uptr kTypeChart = GAME_ADDRESS(0x0077A2A8, 0x007CE428);
// The effect of a type against a type: the number of the result (0 no effect,
// 6 not very effective, 7 normal, 8 super effective), and the flag of the
// inverse battle (a byte that is not 0 when the battle is inverse).
constexpr uptr kTypeMatchup = GAME_ADDRESS(0, 0x007585BC);
constexpr uptr kInverseBattleFlag = GAME_ADDRESS(0, 0x08152AD9);
/// The function that shows the icon of a type in a pane of a battle layout
/// (this, the layout, the pane, the type). ORAS only.
constexpr uptr kShowTypeIcon = GAME_ADDRESS(0, 0x0074B2DC);
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

// The three functions that register the reactions of a held item. The first
// registers the item of a Pokémon (a Pokémon, no result). The second does the
// same after a check of the battle server. The third registers an item for a
// short time (a Pokémon and an item). ORAS only.
constexpr uptr kRegisterItemListener = GAME_ADDRESS(0, 0x007A8124);
constexpr uptr kRegisterItemListenerChecked = GAME_ADDRESS(0, 0x007A1D54);
constexpr uptr kRegisterTemporaryItemListener = GAME_ADDRESS(0, 0x007A821C);
/// The two places of the battle code that compare the item of a Pokémon with
/// the number of items of the game (776). The battle removes an item that is
/// above this number. ORAS only.
constexpr uptr kItemLimitCheck1 = GAME_ADDRESS(0, 0x0076F7C4);
constexpr uptr kItemLimitCheck2 = GAME_ADDRESS(0, 0x007962D4);
/// The check of the second function (a Pokémon number, and the number 5).
/// It returns 1 when the check refuses the registration. ORAS only.
constexpr uptr kItemListenerCheck = GAME_ADDRESS(0, 0x0077526C);

constexpr uptr kLoadAnimation = GAME_ADDRESS(0x006FF9EC, 0x0074E908);
/// The step of the battle scene that shows the trainer of a side and its first
/// Pokémon (the scene, true for the player, true for the fade). Returns true
/// when the step is finished. ORAS only.
constexpr uptr kStartSideEntry = GAME_ADDRESS(0, 0x00750BB8);
/// Adds the 3D model of a trainer to the battle scene. ORAS only.
constexpr uptr kAddTrainerModelToScene = GAME_ADDRESS(0, 0x0074E05C);
/// Loads a sequence of the battle scene and starts it (the scene, the file),
/// and the function that returns true when the sequence is finished (the scene,
/// true to keep the resources). ORAS only.
constexpr uptr kStartSequence = GAME_ADDRESS(0, 0x007510A8);
constexpr uptr kIsSequenceFinished = GAME_ADDRESS(0, 0x00750954);
constexpr uptr kLoadEffect = GAME_ADDRESS(0, 0x0076728C);

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
