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

#include "common.h"
#include "game/constant/item.h"

// Curated item sets handed out in the kaizo starting bag (kaizo_bag.cc).
namespace kaizo {
static constexpr ItemId kStrategicItems[] = {
    ItemId::kChoiceBand, ItemId::kChoiceScarf,
    ItemId::kChoiceSpecs, ItemId::kLifeOrb,
    ItemId::kLeftovers, ItemId::kFocusSash,
    ItemId::kAirBalloon, ItemId::kAssaultVest,
    ItemId::kRockyHelmet, ItemId::kSitrusBerry,
    ItemId::kPowerHerb, ItemId::kFlameOrb,
    ItemId::kEjectButton, ItemId::kScopeLens,
    ItemId::kToxicOrb, ItemId::kGripClaw
};

static constexpr ItemId kEvolutionStones[] = {
    ItemId::kSunStone, ItemId::kMoonStone,
    ItemId::kFireStone, ItemId::kThunderStone,
    ItemId::kWaterStone, ItemId::kLeafStone,
    ItemId::kShinyStone, ItemId::kDuskStone,
    ItemId::kDawnStone, ItemId::kOvalStone,
};

static constexpr ItemId kBalls[] = {
    ItemId::kMasterBall, ItemId::kUltraBall,
    ItemId::kGreatBall, ItemId::kPokeBall,
    ItemId::kSafariBall, ItemId::kNetBall,
    ItemId::kDiveBall, ItemId::kNestBall,
    ItemId::kRepeatBall, ItemId::kTimerBall,
    ItemId::kLuxuryBall, ItemId::kPremierBall,
    ItemId::kDuskBall, ItemId::kHealBall,
    ItemId::kQuickBall, ItemId::kCherishBall,
    ItemId::kFastBall, ItemId::kLevelBall,
    ItemId::kLureBall, ItemId::kHeavyBall,
    ItemId::kLoveBall, ItemId::kFriendBall,
    ItemId::kMoonBall, ItemId::kSportBall,
    ItemId::kParkBall, ItemId::kDreamBall,
};
} // namespace kaizo
