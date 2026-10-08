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
 * @file custom_moves.cc
 * @brief Example moves: Absolute Zero and Solar Flare.
 *
 * Each move has:
 * 1. a data function: power, accuracy, PP, type, effect...
 * 2. an optional animation function,
 * 3. one or more reaction functions and a ReactionTable,
 * 4. one battle::GameExtension::AddMove() call.
 */

#include "custom_battle.h"

#include "battle/constant/moment_kind.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/weather.h"
#include "battle/native/controller.h"
#include "battle/patch/game_extension.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/type.h"
#include "pokemon/native/move_data.h"

namespace battle {
namespace {

// Animation ids that are not moves (is_move = false): the weather animations.
constexpr u32 kHarshSunlightAnimation = 18;
constexpr u32 kHailAnimation = 19;

void AbsoluteZeroHailReaction(Listener* self, Controller* controller,
                              UID owner, s32* local_state) {
  controller->SetWeather(owner, Weather::kHail, ItemId::kNone, true);
}

void SolarFlareSunReaction(Listener* self, Controller* controller, UID owner,
                           s32* local_state) {
  controller->SetWeather(owner, Weather::kHarshSunlight, ItemId::kNone, true);
}

// Absolute Zero: a status move that always hits and always freezes.
void PatchAbsoluteZeroData(pokemon::MoveData& move) {
  move.power = 0;
  move.accuracy = 101;  // 101 = the move never misses.
  move.base_pp = 50;
  move.type = TypeId::kIce;
  move.effect_id = StatusCondition::kFreeze;
  move.effect_rate = 100;
  move.effect_turn_type = 1;
  move.flinch_rate = 0;
  move.category = 1;  // 1 = status only.
  move.damage_category = 0;
}

// Solar Flare: a physical move that always burns and always flinches.
void PatchSolarFlareData(pokemon::MoveData& move) {
  move.power = 0;
  move.accuracy = 100;
  move.base_pp = 50;
  move.type = TypeId::kFire;
  move.effect_id = StatusCondition::kBurn;
  move.effect_rate = 100;
  move.effect_turn_type = 1;
  move.flinch_rate = 100;
  move.category = 4;  // 4 = damage, then a status condition.
  move.damage_category = 1;
}

void UseHailAnimation(u32& id, bool& is_move) {
  id = kHailAnimation;
  is_move = false;
}

void UseSunAnimation(u32& id, bool& is_move) {
  id = kHarshSunlightAnimation;
  is_move = false;
}

const ReactionTable kAbsoluteZeroReactions[] = {
    {MomentKind::kMoveExecutionStart, AbsoluteZeroHailReaction},
};
const ReactionTable kSolarFlareReactions[] = {
    {MomentKind::kMoveExecutionStart, SolarFlareSunReaction},
};

} // namespace

void RegisterCustomMoves() {
  GameExtension::AddMove(
      {kMoveAbsoluteZero, u"Absolute Zero",
       u"Summons a hailstorm\nand instantly freezes the target solid.",
       PatchAbsoluteZeroData, UseHailAnimation, kAbsoluteZeroReactions,
       SIZE(kAbsoluteZeroReactions)});
  GameExtension::AddMove(
      {kMoveSolarFlare, u"Solar Flare",
       u"Summons blinding sunlight\nand instantly leaves the target with a "
       u"severe burn.",
       PatchSolarFlareData, UseSunAnimation, kSolarFlareReactions,
       SIZE(kSolarFlareReactions)});
}

} // namespace battle
