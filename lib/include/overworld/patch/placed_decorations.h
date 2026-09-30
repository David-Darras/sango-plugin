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
#include "overworld/constant/decoration.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"
#include "overworld/native/map_event_data.h"
#include "overworld/native/static_encounter.h"
#include "overworld/native/wild_pokemon.h"
#include "pokemon/patch/model_loader.h"
#include "script/patch/context.h"

namespace overworld {

struct DecorationRequest {
  MapId map_id = MapId::kNone;
  DecorationId decoration = DecorationId::kSmallDesk;
  s16 tile_x = 0;
  s16 tile_z = 0;
  f32 ground_height = 0.0f;
  Facing facing = Facing::kDown;
  bool is_talkable = false;
};

class PlacedDecorations {
  MAKE_SINGLETON(PlacedDecorations)

public:
  static constexpr u32 kMaxDecorations = 8;
  static constexpr u32 kMaxFootprintTiles = 25;
  static constexpr u32 kFirstTalkScript = 31000;

  bool is_collision_enabled = true;
  u32 tall_grass_battle_chance_percent = 25;

  static void Initialize();
  static bool Add(const DecorationRequest& request);
  static bool AddInFrontOfPlayer(DecorationId decoration, bool is_talkable);
  static void Clear();
  static u32 GetCount();
  static void Update();
  static void RemoveModelsBeforeBattle();
  static void OnMapEventsLoaded(MapEventData* events);
  static void ReloadMap();
  static void OnWildPokemonRolled(WildPokemon* pokemons, u32 count);
  static void RunTalkScript(script::Context& script, u32 slot);

private:
  struct Entry {
    DecorationRequest request;
    u8 width = 1;
    u8 depth = 1;
    bool blocks_movement = false;
    bool is_shown = false;
    bool is_tall_grass = false;
    u8 show_attempts = 0;
    u32 saved_tile_mask = 0;
    u32 original_tile_attrs[kMaxFootprintTiles] = {};
    pokemon::LoadedModel model;
  };

  static constexpr u32 kShowDelayFrames = 20;
  static constexpr u32 kMaxShowAttempts = 5;
  static constexpr u32 kMaxTalkEvents = 64;
  static constexpr u32 kTallGrassStepDelayFrames = 8;
  static constexpr u32 kBattleTimeoutFrames = 1800;
  static constexpr u32 kMaxExits = 32;
  static constexpr s32 kExitSafeDistance = 1;

  static bool ReadDecorationInfo(u16 index, u8* width, u8* depth,
                                 bool* blocks_movement);
  static void ShowModel(Entry& entry);
  static void HideModel(Entry& entry);
  static void DiscardModel(Entry& entry);
  static bool IsNearExit(MapId map);
  static bool IsOnTallGrass(s32 tile_x, s32 tile_z);
  static void UpdateTallGrass();
  static void StartTallGrassBattle();
  static void RestoreBorrowedEncounter();
  static void ApplyTileChanges(Entry& entry);
  static void RestoreTiles(Entry& entry);

  Entry entries_[kMaxDecorations];
  u32 count_ = 0;
  u32 frame_ = 0;
  u32 map_load_frame_ = 0;
  SignEvent talk_events_[kMaxTalkEvents];
  s32 tall_grass_tile_x_ = -1;
  s32 tall_grass_tile_z_ = -1;
  u32 step_countdown_ = 0;
  bool is_tall_grass_battle_pending_ = false;
  bool is_battle_starting_ = false;
  bool is_battle_seen_ = false;
  u32 battle_start_frame_ = 0;
  bool has_saved_encounter_ = false;
  StaticEncounter saved_encounter_;
  MapId exit_map_ = MapId::kNone;
  u32 exit_count_ = 0;
  s16 exit_tile_x_[kMaxExits] = {};
  s16 exit_tile_z_[kMaxExits] = {};
};
} // namespace overworld
