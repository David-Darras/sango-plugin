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

#include "core/types.h"
#include "overworld/constant/map.h"
#include "overworld/native/character_placement.h"
#include "overworld/native/sign_event.h"
#include "overworld/native/trigger_event.h"
#include "overworld/native/warp_event.h"

namespace overworld {

constexpr u32 kMaxCharactersPerMap = 32;

/// The event data of one map (the game's EventData): the signs, characters,
/// warps and triggers placed on it.
struct MapEventData {
  MapId map_id;
  u16 sign_count;
  u16 character_count;
  u16 warp_count;
  u16 trigger_count;
  u16 interrupt_count;
  SignEvent* signs;
  CharacterPlacement* characters;
  WarpEvent* warps;
  TriggerEvent* triggers;
  TriggerEvent* interrupts;
  u16 _0;
  u16 character_capacity;
};

static_assert(sizeof(MapEventData) == 0x24 &&
              offsetof(MapEventData, characters) == 0x10 &&
              offsetof(MapEventData, character_capacity) == 0x22,
              "MapEventData must match the game's event data layout");

} // namespace overworld
