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
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/native/character_manager.h"
#include "overworld/native/character_placement.h"
#include "overworld/native/map_event_data.h"
#include "overworld/native/model_appearance.h"
#include "script/constant/script.h"

namespace overworld {

struct MapCharacterRequest {
  MapId map_id = MapId::kNone;
  ModelId model_id = ModelId::kNone;
  ScriptId script_id = ScriptId::kNone;
  u16 tile_x = 0;
  u16 tile_z = 0;
  f32 height = 0.0f;
  Facing facing = Facing::kDown;
  u16 movement_id = 0;
  u16 hide_when_flag_set = 0;
};

class MapCharacter {
  MAKE_SINGLETON(MapCharacter)

public:
  static constexpr u32 kMaxRequests = 16;
  bool is_logging_enabled = false;

  static void Initialize();
  static bool Add(const MapCharacterRequest& request);
  static void Clear();
  static bool Empty(MapId map_id);
  static u32 GetCount();
  static void ReloadMapAfterBattleWith(u16 trainer_id);
  static void Update();

private:
  static constexpr u16 kFirstTrainerScript = 3000;
  static constexpr u32 kMaxEmptiedMaps = 8;

  static void ReloadCurrentMap();
  static u32 LoadMapCharacters(MapEventData* events, u32 buffer_id);
  static void MoveGraftedEvents(MapEventData* events);
  static void CompleteRegionModelList(CharacterManager* manager,
                                      u32 player_sex,
                                      const CharacterPlacement* placements,
                                      u32 placement_count);
  void LogShippedCharacters(const MapEventData* events) const;
  void PlaceCharacters(MapEventData* events);
  bool IsEmptied(MapId map_id) const;
  static void BuildPlacement(const MapCharacterRequest& request, u16 local_id,
                             CharacterPlacement* out);
  void AddMissingModels(CharacterManager* manager,
                        const CharacterPlacement* placements,
                        u32 placement_count);
  static bool Contains(const ModelAppearance* models, u32 count,
                       ModelId model_id);
  static bool LoadAppearance(const CharacterManager* manager,
                             ModelId model_id, ModelAppearance* out);

  MapCharacterRequest requests_[kMaxRequests];
  u32 request_count_ = 0;
  bool is_reload_armed_ = false;
  bool has_resting_spot_ = false;
  f32 resting_x_ = 0.0f;
  f32 resting_z_ = 0.0f;
  CharacterPlacement placements_[kMaxCharactersPerMap];
  MapId emptied_maps_[kMaxEmptiedMaps];
  u32 emptied_count_ = 0;
};
} // namespace overworld
