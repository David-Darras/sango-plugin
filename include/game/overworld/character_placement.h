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

namespace overworld {
enum class Facing : u16 {
  kUp = 0,
  kDown = 1,
  kLeft = 2,
  kRight = 3,
};

enum class PositionKind : u32 {
  kTileGrid = 0,
  kRail = 1,
};

struct CharacterPosition {
  PositionKind kind;
  u16 tile_x;
  u16 tile_z;
  f32 height;
};

struct CharacterPlacement {
  u16 local_id;
  u16 model_id;
  u16 movement_id;
  u16 interaction_kind;
  u16 hide_when_flag_set;
  u16 script_id;
  u16 facing;
  u16 param0;
  u16 param1;
  u16 param2;
  s16 trigger_x;
  s16 trigger_z;
  s16 trigger_width;
  s16 trigger_depth;
  u16 displayed_on_map_id;
  u16 owned_by_map_id;
  u16 map_id;
  u16 alias_id;
  CharacterPosition position;
};

static_assert(sizeof(CharacterPlacement) == 48,
              "CharacterPlacement must match the game's map event data layout");

struct ModelAppearance {
  u16 model_id;
  u8 draw_kind;
  u8 draw_code;
  u16 skeleton_preset;
  u8 shadow_kind;
  u8 footprint_kind;
  u8 reflection_kind;
  u8 gender;
  u8 width;
  u8 depth;
  s8 offset_x;
  s8 offset_y;
  s8 offset_z;
  u8 uses_outfit;
  u8 keeps_outfit_in_memory;
  u8 outline_kind;
  u16 outfit_pattern;
  u8 reserved[4];
};

static_assert(sizeof(ModelAppearance) == 24,
              "ModelAppearance must match the game's model list layout");

constexpr u32 kMaxCharactersPerMap = 26;
constexpr u32 kMaxModelsPerRegion = 33;

namespace map_event_offsets {
constexpr u32 kMapId = 0x00;
constexpr u32 kCharacterCount = 0x04;
constexpr u32 kCharacters = 0x10;
constexpr u32 kCharacterCapacity = 0x22;
} // namespace map_event_offsets

namespace character_manager_offsets {
constexpr u32 kModelParamArchive = 14004;
constexpr u32 kModelListSetupStage = 13994;
constexpr u32 kModelCountOriginal = 14020;
constexpr u32 kModelCount = 14024;
constexpr u32 kModelList = 14028;
} // namespace character_manager_offsets
} // namespace overworld