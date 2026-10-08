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
 * @file custom_battle.h
 * @brief The new moves and the new abilities of the overlay.
 *
 * These moves and abilities are examples. Use them as a model for your
 * own product. See docs/tutorials/04-add-a-move.md and
 * docs/tutorials/05-add-an-ability.md.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/move.h"

/// @name The ids of the new abilities (the game uses ids 1 to 191).
/// @{
constexpr AbilityId kAbilityToxicDrizzle = static_cast<AbilityId>(255);
constexpr AbilityId kAbilityRadioactiveDrizzle = static_cast<AbilityId>(254);
constexpr AbilityId kAbilityRealityWarp = static_cast<AbilityId>(253);
constexpr AbilityId kAbilityElectricSurge = static_cast<AbilityId>(252);
constexpr AbilityId kAbilityPsychicSurge = static_cast<AbilityId>(251);
constexpr AbilityId kAbilityGrassySurge = static_cast<AbilityId>(250);
constexpr AbilityId kAbilityMistySurge = static_cast<AbilityId>(249);
constexpr AbilityId kAbilityBeastBoost = static_cast<AbilityId>(248);
/// @}

/// @name The ids of the new moves (the game uses ids 1 to 621).
/// @{
constexpr MoveId kMoveAbsoluteZero = static_cast<MoveId>(863);
constexpr MoveId kMoveSolarFlare = static_cast<MoveId>(864);
/// @}

namespace battle {
/// Adds the new abilities of the overlay to the game.
void RegisterCustomAbilities();
/// Adds the new moves of the overlay to the game.
void RegisterCustomMoves();
} // namespace battle
