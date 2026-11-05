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
 * @code
 * overworld::MapCharacter::Add(
 *     overworld::Character(MapId::kLittlerootTown, ModelId::kResearcherMale)
 *         .At(47, 77, 6)
 *         .WithFacing(overworld::Facing::kDown)
 *         .WithScript(kCandyGiver));
 * @endcode
 *
 * @see docs/tutorials/09-write-an-overworld-script.md
 */

#pragma once

#include "common.h"
#include "core/fixed_list.h"
#include "battle/constant/trainer.h"
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
  /// A trainer of the game: the character is this trainer. When the player
  /// walks in front of it or talks to it, the trainer talks and starts the
  /// battle, like the trainers of the game. After a win, it battles no more.
  /// battle::TrainerTeams can give it a new team. `script_id` is then not
  /// used. TrainerId::kNone: a normal character.
  TrainerId trainer_id = TrainerId::kNone;
  /// The number of tiles that the trainer sees in front of it (1 to 10).
  u8 sight = 3;
  /// Any address that names the code that added the character, for
  /// MapCharacter::RemoveByOwner(). Null: no owner.
  const void* owner = nullptr;
};

/**
 * @brief Describes a character to add to a map, one part at a time.
 *
 * Each function changes one part, and the parts that you do not give keep
 * the default values of MapCharacterRequest. Give the result to
 * MapCharacter::Add().
 */
class Character {
public:
  /// A character on one map, with a model.
  Character(MapId map_id, ModelId model_id) {
    request_.map_id = map_id;
    request_.model_id = model_id;
  }
  /// A character on all the maps, with a model.
  explicit Character(ModelId model_id) {
    request_.model_id = model_id;
    request_.is_everywhere = true;
  }

  /// The position in tiles, and the height (Tile X, Tile Z and Height in
  /// Overworld > Location).
  Character& At(u16 tile_x, u16 tile_z, f32 height = 0.0f) {
    request_.tile_x = tile_x;
    request_.tile_z = tile_z;
    request_.height = height;
    return *this;
  }
  /// The direction.
  Character& WithFacing(Facing facing) {
    request_.facing = facing;
    return *this;
  }
  /// The script when the player talks to the character.
  Character& WithScript(ScriptId script_id) {
    request_.script_id = script_id;
    return *this;
  }
  /// Makes the character a trainer of the game that sees `sight` tiles (1 to
  /// 10) in front of it.
  Character& AsTrainer(TrainerId trainer_id, u8 sight = 3) {
    request_.trainer_id = trainer_id;
    request_.sight = sight;
    return *this;
  }
  /// The movement code. 0 is the default value.
  Character& WithMovement(u16 movement_id) {
    request_.movement_id = movement_id;
    return *this;
  }
  /// The character is not there when this event flag is set.
  Character& HiddenWhenFlagSet(u16 event_flag) {
    request_.hide_when_flag_set = event_flag;
    return *this;
  }
  /// The character is there only when this switch of the product is true.
  Character& EnabledBy(const bool* is_enabled) {
    request_.is_enabled = is_enabled;
    return *this;
  }

  /// Gives the description.
  const MapCharacterRequest& GetRequest() const { return request_; }

private:
  MapCharacterRequest request_;
};

/// Adds characters to the maps (128 at most).
class MapCharacter {
  MAKE_SINGLETON(MapCharacter)

public:
  /// The number of characters that the product can add, on all the maps.
  /// A map shows 32 characters at most: the characters of the game first.
  static constexpr u32 kMaxRequests = 128;
  bool is_logging_enabled = false;

  static void Initialize();
  /// Adds a character. Returns false when the list is full.
  static bool Add(const MapCharacterRequest& request);
  /// Adds a character of overworld::Character.
  static bool Add(const Character& character) {
    return Add(character.GetRequest());
  }
  /// Removes all the added characters.
  static void Clear();
  /// Removes the characters that `owner` added (MapCharacterRequest::owner).
  /// The map shows the change at its next load.
  static void RemoveByOwner(const void* owner);
  /// Loads the current map again when the player moves, like a warp: the
  /// map then shows the new characters and their models. Call it after a
  /// change of the characters while the player is on a map.
  static void ReloadMapWhenPlayerMoves();
  /// Removes the characters of the game from a map (64 maps at most).
  static bool Empty(MapId map_id);
  /// Returns the number of added characters.
  static u32 GetCount();
  /// Returns an added character (index from 0 to GetCount() - 1).
  static const MapCharacterRequest& GetRequest(u32 index) {
    return GetInstance().requests_[index < kMaxRequests ? index : 0];
  }
  /// Returns the placement (script, movement...) of the character with this
  /// id on the current map, or null (the player has none).
  static CharacterPlacement* FindPlacement(u16 local_id);
  /// Returns the id on the map of the character with this script.
  static u16 GetLocalId(ScriptId script_id);
  /// Loads the map again after a battle with this trainer.
  static void ReloadMapAfterBattleWith(u16 trainer_id);
  /// Called at each frame by plugin::UpdateFrame().
  static void Update();

private:
  static constexpr u16 kFirstTrainerScript = 3000;
  /// The kind of the characters that are trainers and see in front of them.
  static constexpr u16 kTrainerKind = 1;
  static constexpr u32 kMaxEmptiedMaps = 64;

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
  // Returns true when the map is in the current region of the game.
  static bool IsInRegion(MapId map_id);
  static bool Contains(const ModelAppearance* models, u32 count,
                       ModelId model_id);
  static bool LoadAppearance(const CharacterManager* manager,
                             ModelId model_id, ModelAppearance* out);

  MapEventData* loaded_events_ = nullptr; ///< The events of the last map.
  core::FixedList<MapCharacterRequest, kMaxRequests> requests_;
  bool is_reload_armed_ = false;
  MapId arrival_map_ = MapId::kNone;
  s32 arrival_tile_x_ = -1;
  s32 arrival_tile_z_ = -1;
  f32 world_per_tile_ = 18.0f;
  bool has_resting_spot_ = false;
  f32 resting_x_ = 0.0f;
  f32 resting_z_ = 0.0f;
  // The game keeps the events of two maps when the player walks from one map
  // to the next without a loading screen: each map needs its own list.
  CharacterPlacement placements_[2][kMaxCharactersPerMap];
  u32 placement_buffer_ = 0;
  MapId emptied_maps_[kMaxEmptiedMaps];
  u32 emptied_count_ = 0;
};
} // namespace overworld
