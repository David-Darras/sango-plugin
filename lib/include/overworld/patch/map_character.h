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
 * @file map_character.h
 * @brief Adds characters with C++ scripts to the maps.
 *
 * @see docs/tutorials/09-write-an-overworld-script.md
 */

#pragma once

#include "common.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/native/character_manager.h"
#include "overworld/native/character_placement.h"
#include "overworld/native/map_event_data.h"
#include "overworld/native/model_appearance.h"
#include "overworld/native/position.h"
#include "script/constant/script.h"

namespace core {
class GameManager;
}

namespace overworld {

/// A character to add to a map.
struct MapCharacterRequest {
  MapId map_id = MapId::kNone; ///< The map.
  ModelId model_id = ModelId::kNone; ///< The model of the character.
  ScriptId script_id = ScriptId::kNone; ///< The script when the player talks to the character.
  u16 tile_x = 0; ///< The X position in tiles.
  u16 tile_z = 0; ///< The Z position in tiles.
  f32 height = 0.0f; ///< The height (Tile Y in the Player page of the overlay).
  Facing facing = Facing::kDown; ///< The direction.
  u16 movement_id = 0; ///< The movement code. 0 is the default value.
  u16 hide_when_flag_set = 0; ///< An event flag. When it is set, the character is not there. 0: always there.
  bool is_everywhere = false; ///< true: the character is on all the maps.
  u16 local_id = 0xFFFF; ///< The id on the map. 0xFFFF: the library selects it.
  /// A switch of the product. When it is false, the character is not on the
  /// maps. The change shows at the next map load. Null: always there.
  const bool* is_enabled = nullptr;
};

/// Adds characters to the maps (16 at most).
class MapCharacter {
  MAKE_SINGLETON(MapCharacter)

public:
  static constexpr u32 kMaxRequests = 16;
  bool is_logging_enabled = false;

  static void Initialize();
  /// Adds a character. Returns false when the list is full.
  static bool Add(const MapCharacterRequest& request);
  /// Removes all the added characters.
  static void Clear();
  /// Removes the characters of the game from a map (8 maps at most).
  static bool Empty(MapId map_id);
  /// Returns the number of added characters.
  static u32 GetCount();
  /// Returns the id on the map of the character with this script.
  static u16 GetLocalId(ScriptId script_id);
  /// Loads the map again after a battle with this trainer.
  static void ReloadMapAfterBattleWith(u16 trainer_id);
  /// Called at each frame by plugin::UpdateFrame().
  static void Update();

private:
  static constexpr u16 kFirstTrainerScript = 3000;
  static constexpr u32 kMaxEmptiedMaps = 8;

  static void ReloadCurrentMap();
#ifdef GAME_ORAS
  static void ChangeMapHook(core::GameManager* manager, MapId map_id,
                            const Position* position, Facing facing,
                            u8 p0, bool p1, s32 p2, s32 p3, s32 p4,
                            bool p5);
#endif
  static u32 LoadMapCharacters(MapEventData* events, u32 buffer_id);
  static void MoveGraftedEvents(MapEventData* events);
  static void CompleteRegionModelList(CharacterManager* manager,
                                      u32 player_sex,
                                      const CharacterPlacement* placements,
                                      u32 placement_count,
                                      void* player_outfit,
                                      void* character_outfits);
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
  MapId arrival_map_ = MapId::kNone;
  s32 arrival_tile_x_ = -1;
  s32 arrival_tile_z_ = -1;
  f32 world_per_tile_ = 18.0f;
  bool has_resting_spot_ = false;
  f32 resting_x_ = 0.0f;
  f32 resting_z_ = 0.0f;
  CharacterPlacement placements_[kMaxCharactersPerMap];
  MapId emptied_maps_[kMaxEmptiedMaps];
  u32 emptied_count_ = 0;
};
} // namespace overworld
