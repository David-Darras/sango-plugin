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
 * @brief The new moves, abilities and types of the overlay.
 *
 * These moves, abilities and types are examples. Use them as a model for your
 * own product. See docs/tutorials/04-add-a-move.md and
 * docs/tutorials/05-add-an-ability.md.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/type.h"

namespace ui {
class MainApplication;
}

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

/// @name The numbers of the new types (the game uses 0 to 17 and 18 for "no
/// type").
/// @{
constexpr TypeId kTypeCosmic = static_cast<TypeId>(19);
constexpr TypeId kTypeSound = static_cast<TypeId>(20);
constexpr TypeId kTypeLight = static_cast<TypeId>(21);
constexpr TypeId kTypePlasma = static_cast<TypeId>(22);
constexpr TypeId kTypeDream = static_cast<TypeId>(23);
constexpr TypeId kTypeTime = static_cast<TypeId>(24);
/// @}

/// @name The ids of the new moves (the game uses ids 1 to 621).
/// @{
constexpr MoveId kMoveAbsoluteZero = static_cast<MoveId>(863);
constexpr MoveId kMoveSolarFlare = static_cast<MoveId>(864);
constexpr MoveId kMovePrismLance = static_cast<MoveId>(865);
constexpr MoveId kMoveSonicRequiem = static_cast<MoveId>(866);
constexpr MoveId kMoveIonStorm = static_cast<MoveId>(867);
constexpr MoveId kMoveDreamfall = static_cast<MoveId>(868);
/// @}

namespace battle {
/// Adds the new abilities of the overlay to the game.
void RegisterCustomAbilities();
/// Adds the new moves of the overlay to the game.
void RegisterCustomMoves();
/// Adds the four moves of the new types (Light, Sound, Plasma and Dream).
void RegisterTypeMoves();
/// Adds the animations of these four moves.
void RegisterTypeMovesAnimations();
/// Adds the new types of the overlay to the game.
void RegisterCustomTypes();
/// Gives the new types to some moves and species of the game (a team to test
/// them). Call it once at the start.
void PatchTypedPokemon();
/// Gives the party of the player the six Pokemon of the new types, with their
/// moves, at level 100 (the page "Pokemon" has a button for it).
void GiveTypeTeam();
/// Adds the section "Type Test" at the end of the page "Pokemon".
void AddTypeTestEntries(ui::MainApplication& app);
} // namespace battle
