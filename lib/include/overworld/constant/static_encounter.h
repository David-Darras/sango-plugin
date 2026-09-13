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

#include <types.h>

namespace overworld {

/// How the game runs a scripted encounter (StaticEncounter).
enum class StaticEncounterKind : u16 {
  kNormal = 0, ///< A regular random wild encounter, with no visible sprite
  kOverworldEncounter = 1, ///< A regular Pokémon visible on the map
  kLegendary = 2,
  ///< A legendary Pokémon - uses a distinct encounter message and only fights once
  kLegendaryEndless = 3,
  ///< Same as kLegendary, but stays on the map and can be re-fought after being defeated or caught
  kRescueEvent = 4,
  ///< The early-game scripted battle where the player saves the professor
  ///< from a wild Pokémon (a level 3 male Poochyena) - running away is
  ///< disabled and the messages differ
  kLegendaryUnlosable = 5,
  ///< Same as kLegendary, but the battle can't be lost
};

/// Which of the species' abilities a scripted encounter gets.
enum class AbilitySlot : u16 {
  kRandom = 0,
  kFirst = 1,
  kSecond = 2,
  kHidden = 3,
};

/// Rows of the game's scripted encounter table.
enum class StaticEncounterId : u32 {
  kPoochyena = 0, ///< The rescue-event encounter, see StaticEncounterKind
  kThundurus = 1,
  kTornadus = 2,
  kLandorus = 3,
  kDialga = 4,
  kPalkia = 5,
  kGiratina = 6,
  kEntei = 7,
  kRaikou = 8,
  kSuicune = 9,
  kTerrakion = 10,
  kCobalion = 11,
  kVirizion = 12,
  kUxie = 13,
  kAzelf = 14,
  kMesprit = 15,
  kReshiram = 16,
  kZekrom = 17,
  kKyurem = 18,
  kHoOh = 19,
  kLugia = 20,
  kVoltorb1 = 21,
  kVoltorb2 = 22,
  kVoltorb3 = 23,
  kVoltorb4 = 24,
  kSpiritomb = 25,
  kKyogre = 26,
  kGroudon = 27,
  kRayquaza = 28,
  kDeoxys1 = 29,
  kTaillow = 30,
  kWingull = 31,
  kPelipper = 32,
  kSwablu = 33,
  kBraviary = 34,
  kDrifblim = 35,
  kMurkrow = 36,
  kHeatran = 37,
  kCresselia = 38,
  kKecleon1 = 39,
  kKecleon2 = 40,
  kKecleon3 = 41,
  kKecleon4 = 42,
  kKecleon5 = 43,
  kKecleon6 = 44,
  kElectrode1 = 45,
  kElectrode2 = 46,
  kElectrode3 = 47,
  kElectrode4 = 48,
  kKyogrePrimal = 49,
  kGroudonPrimal = 50,
  kRegirock = 51,
  kRegice = 52,
  kRegisteel = 53,
  kRegigigas = 54,
  kWurmple = 55,
  kLatios = 56,
  kLatias = 57,
  kDeoxys2 = 58,
};

} // namespace overworld

using overworld::StaticEncounterKind;
using overworld::AbilitySlot;
using overworld::StaticEncounterId;
