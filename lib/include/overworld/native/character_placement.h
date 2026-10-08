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
 * @file character_placement.h
 * @brief One character of a map.
 */

#pragma once

#include "core/types.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/native/character_position.h"
#include "script/constant/script.h"

namespace overworld {

/// One character of a map: model, script, position.
struct CharacterPlacement {
  u16 local_id;
  ModelId model_id; ///< The model.
  u16 movement_id; ///< The movement code.
  u16 interaction_kind;
  u16 hide_when_flag_set; ///< An event flag. When it is set, the character is not there.
  ScriptId script_id; ///< The script when the player talks to the character.
  Facing facing; ///< The direction.
  u8 _0;
  u16 param0;
  u16 param1;
  u16 param2;
  s16 trigger_x;
  s16 trigger_z;
  s16 trigger_width;
  s16 trigger_depth;
  MapId displayed_on_map_id;
  MapId owned_by_map_id;
  MapId map_id;
  u16 alias_id;
  CharacterPosition position;
};

static_assert(sizeof(CharacterPlacement) == 48,
              "CharacterPlacement must match the game's map event data layout");

} // namespace overworld
