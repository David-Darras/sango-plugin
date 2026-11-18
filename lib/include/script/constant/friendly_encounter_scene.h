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
 * @file friendly_encounter_scene.h
 * @brief The values that the scripts use for the friendly encounter scene.
 */

#pragma once

#include <types.h>

namespace script {

/// The friendly encounter scene, as a script writes it.
enum class FriendlyEncounterScene : u8 {
  kRoute102Wingull = 0,
  kRoute102Beautifly = 1,
  kRoute104Wingull1 = 2,
  kRoute104Wingull2 = 3,
  kRoute104Wingull3 = 4,
  kRoute105Wingull1 = 5,
  kRoute105Wingull2 = 6,
  kRoute107Wingull = 7,
  kRoute108Wingull1 = 8,
  kRoute108Wingull2 = 9,
  kRoute109Wingull1 = 10,
  kRoute109Wingull2 = 11,
  kRoute109Wingull3 = 12,
  kRoute114Swablu = 13,
  kRoute124Wingull1 = 14,
  kRoute124Wingull2 = 15,
  kRoute124UnderwaterLuvdisc1 = 16,
  kRoute124UnderwaterLuvdisc2 = 17,
  kRoute124UnderwaterCorsola = 18,
  kRoute126UnderwaterLuvdisc1 = 19,
  kRoute126UnderwaterLuvdisc2 = 20,
  kRoute126UnderwaterLuvdisc3 = 21,
  kRoute127UnderwaterRelicanth = 22,
  kRoute127UnderwaterChinchou1 = 23,
  kRoute127UnderwaterChinchou2 = 24,
  kRoute128UnderwaterRelicanth = 25,
  kRoute128UnderwaterCorsola = 26,
  kRoute128UnderwaterChinchou = 27,
};

} // namespace script
