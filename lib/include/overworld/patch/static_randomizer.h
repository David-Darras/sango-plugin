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
#include "overworld/constant/static_encounter.h"

namespace core {
class GameManager;
}

namespace overworld {

class StaticRandomizer {
  MAKE_SINGLETON(StaticRandomizer)

public:
  bool randomize_species = false;

  static void Initialize();
  static void PatchLoad();

private:
  static void RandomizeSpecies(StaticEncounterId id);
  static s32 CallStaticEncounterHook(core::GameManager* man, StaticEncounterId id,
                                     u32 p3, u32 p4);
};

} // namespace overworld
