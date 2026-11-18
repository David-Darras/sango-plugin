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
 * @file word_kind.h
 * @brief The values that the scripts use for the word kind.
 */

#pragma once

#include <types.h>

namespace script {

/// The word kind, as a script writes it.
enum class WordKind : u8 {
  kWord = 0,
  kPlayerName = 1,
  kPlayerNickname = 2,
  kPrefixPlayerNickname = 3,
  kRivalName = 4,
  kPokemonSpeciesName = 5,
  kItemName = 6,
  kMoveNameTMOrHMItem = 7,
  kBagPocketName = 8,
  kNumber = 9,
  kMoveName = 10,
  kNicknamePokemonParty = 11,
  kNicknamePokemonInBox = 12,
  kTypeName = 13,
  kNatureName = 14,
  kBoxName = 15,
  kTrainerClassName = 16,
  kTrainerName = 17,
  kPlaceName = 18,
  kWeatherName = 19,
  kAssistPowerName = 20,
  kRibbonName = 21,
  kSecretBaseGroupName = 23,
  kSecretBasePassword = 24,
  kSecretBaseWordWhenHappy = 25,
  kSecretBaseWordEncourage = 26,
  kSecretBaseWordOnBlackboard = 27,
  kSecretBaseWordOnPaperBall = 28,
  kSecretBaseOwnerName = 29,
  kSecretBaseDisplayedName = 30,
  kOwnSecretBaseGroupName = 33,
  kOwnSecretBasePassword = 34,
  kOwnSecretBaseWordWhenHappy = 35,
  kOwnSecretBaseWordEncourage = 36,
  kOwnSecretBaseWordOnBlackboard = 37,
  kOwnSecretBaseWordOnPaperBall = 38,
  kOwnSecretBaseOwnerName = 39,
  kOwnSecretBaseDisplayedName = 40,
};

} // namespace script
