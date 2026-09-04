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
static constexpr ItemID kStrategicItems[] = {
    ItemID::kChoiceBand, ItemID::kChoiceScarf,
    ItemID::kChoiceSpecs, ItemID::kLifeOrb,
    ItemID::kLeftovers, ItemID::kFocusSash,
    ItemID::kAirBalloon, ItemID::kAssaultVest,
    ItemID::kRockyHelmet, ItemID::kSitrusBerry,
    ItemID::kPowerHerb, ItemID::kFlameOrb,
    ItemID::kEjectButton, ItemID::kScopeLens,
    ItemID::kToxicOrb, ItemID::kGripClaw
};

static constexpr ItemID kEvolutionStones[] = {
    ItemID::kSunStone, ItemID::kMoonStone,
    ItemID::kFireStone, ItemID::kThunderStone,
    ItemID::kWaterStone, ItemID::kLeafStone,
    ItemID::kShinyStone, ItemID::kDuskStone,
    ItemID::kDawnStone, ItemID::kOvalStone,
};

static constexpr ItemID kBalls[] = {
    ItemID::kMasterBall, ItemID::kUltraBall,
    ItemID::kGreatBall, ItemID::kPokeBall,
    ItemID::kSafariBall, ItemID::kNetBall,
    ItemID::kDiveBall, ItemID::kNestBall,
    ItemID::kRepeatBall, ItemID::kTimerBall,
    ItemID::kLuxuryBall, ItemID::kPremierBall,
    ItemID::kDuskBall, ItemID::kHealBall,
    ItemID::kQuickBall, ItemID::kCherishBall,
    ItemID::kFastBall, ItemID::kLevelBall,
    ItemID::kLureBall, ItemID::kHeavyBall,
    ItemID::kLoveBall, ItemID::kFriendBall,
    ItemID::kMoonBall, ItemID::kSportBall,
    ItemID::kParkBall, ItemID::kDreamBall,
};
} // namespace kaizo
