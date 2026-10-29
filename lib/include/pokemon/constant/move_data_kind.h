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
 * @file move_data_kind.h
 * @brief The values of the fields of pokemon::MoveData: the category of a
 *        move, its damage category, its target, the stats it changes and how
 *        long its status condition stays.
 */

#pragma once

#include <types.h>

namespace pokemon {

/// What a move does. It selects the part of the battle engine that runs the move.
/// A move that must hurt the target needs a damage kind: a move with the kind
/// kEffect changes stats and deals no damage, whatever its power is.
enum class MoveCategory : u8 {
  kDamage = 0, ///< Damage only.
  kStatus = 1, ///< A status condition only.
  kEffect = 2, ///< A stat change only.
  kRecover = 3, ///< Gives HP back.
  kDamageStatus = 4, ///< Damage, then a status condition.
  kEffectStatus = 5, ///< A stat change and a status condition.
  kDamageEffect = 6, ///< Damage, then a stat change of the target.
  kDamageEffectUser = 7, ///< Damage, then a stat change of the user.
  kDrain = 8, ///< Damage, and the user gets HP back.
  kOneHitKo = 9, ///< Knocks the target out in one hit.
  kFieldEffect = 10, ///< An effect on the field.
  kSideEffect = 11, ///< An effect on one side of the field.
};

/// How the game calculates the damage of a move.
enum class DamageCategory : u8 {
  kStatus = 0, ///< No damage.
  kPhysical = 1, ///< Attack against Defense.
  kSpecial = 2, ///< Sp. Atk against Sp. Def.
};

/// The Pokémon that a move hits.
enum class MoveTarget : u8 {
  kOtherSelect = 0, ///< One other Pokémon, the player selects it.
  kFriendUserSelect = 1, ///< One ally or the user, the player selects it.
  kFriendSelect = 2, ///< One ally, the player selects it.
  kEnemySelect = 3, ///< One opponent, the player selects it.
  kOtherAll = 4, ///< All the other Pokémon.
  kEnemyAll = 5, ///< All the opponents.
  kFriendAll = 6, ///< All the allies.
  kUser = 7, ///< The user.
  kAll = 8, ///< All the Pokémon.
  kEnemyRandom = 9, ///< One opponent at random.
  kField = 10, ///< The field.
  kSideEnemy = 11, ///< The side of the opponents.
  kSideFriend = 12, ///< The side of the user.
};

/// A stat that a move changes.
enum class MoveStat : u8 {
  kNone = 0,
  kAttack = 1,
  kDefense = 2,
  kSpecialAttack = 3,
  kSpecialDefense = 4,
  kSpeed = 5,
  kAccuracy = 6,
  kEvasion = 7,
};

/// How long the status condition of a move stays.
enum class EffectTurnType : u8 {
  kNone = 0,
  kPermanent = 1,
  kTurnBased = 2,
  kWhileUserPresent = 3,
  kTurnOrUserLeave = 4,
};

} // namespace pokemon

using pokemon::DamageCategory;
using pokemon::EffectTurnType;
using pokemon::MoveCategory;
using pokemon::MoveStat;
using pokemon::MoveTarget;
