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

#include <cstddef>

#include "common.h"
#include "battle/constant/background.h"
#include "battle/constant/encounter_animation.h"
#include "battle/constant/ground.h"
#include "overworld/constant/ability_slot.h"
#include "overworld/constant/static_encounter.h"
#include "overworld/constant/static_encounter_kind.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender_roll.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/shiny_roll.h"
#include "pokemon/constant/species.h"

namespace overworld {
/// One row of the game's scripted encounter table (legendaries, the rescue
/// event...): what to fight and how to stage the battle.
struct StaticEncounter {
  SpeciesId species;
  FormId form;
  u8 level;

  ItemId item;
  ShinyRoll is_shiny : 2;
  GenderRoll gender : 2;
  AbilitySlot ability : 3;
  StaticEncounterKind kind : 3;
  u16 : 6;

  BackgroundId background;
  GroundId ground;
  EncounterAnimationId animation;
  u8 _0;

  STATIC_INLINE StaticEncounter& GetInstance(StaticEncounterId id) {
    auto* table = (StaticEncounter*)pokemon::address::kStaticEncounterTable;
    return table[static_cast<u32>(id)];
  }
};

static_assert(sizeof(StaticEncounter) == 12 &&
              offsetof(StaticEncounter, background) == 8,
              "StaticEncounter must match the game's encounter table layout");
} // namespace overworld
