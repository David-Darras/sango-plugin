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

enum class BattleTrainer : u16 {
  kNone = 0,

  // ROUTE 103
  kRoute103Brendan1 = 1,
  kRoute103Brendan2 = 2,
  kRoute103Brendan3 = 3,
  kRoute103May1 = 4,
  kRoute103May2 = 5,
  kRoute103May3 = 6,

  // ROUTE 102
  kRoute102Kid1 = 7,
  kRoute102Kid2 = 8,
  kRoute102Kid3 = 301,
  kRoute102Girl = 10,

  // ROUTE 104 DOWN
  kRoute104YoungsterBilly = 16,
  kRoute104RichBoyWinston = 639,

  // PETALBURG WOODS
  kPetalburgWoodsBugCatcherLyle = 9,
  kPetalburgWoodsTeamAquaGrunt = 20,
  kPetalburgWoodsBugCatcherJames = 302,

  // BATTLE ROUTE 104 UP
  kRoute104LadyCindy = 640,
  kRoute104LassHaley = 17,
  kRoute104TwinsGinaAndMia = 275,
  kRoute104FishermanIvan = 14,

  // RUSTBORO CITY
  kRustboroCityYoungsterJosh = 562,
  kRustboroCityYoungsterTommy = 22,
  kRustboroCitySchoolkidGeorgia = 667,
  kRustboroCityLeaderRoxanne = 561,
};
