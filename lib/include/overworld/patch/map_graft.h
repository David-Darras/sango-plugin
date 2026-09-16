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
#include "overworld/native/character_placement.h"
#include "overworld/native/map_event_data.h"
#include "overworld/native/map_settings.h"

namespace overworld {
struct WorldLayout;

struct MapGraftRequest {
  MapId anchor = MapId::kNone;
  Facing side = Facing::kUp;
  MapId map = MapId::kNone;
  s32 shift = 0;
};

struct MapLinkRequest {
  MapId from = MapId::kNone;
  Facing side = Facing::kUp;
  MapId to = MapId::kNone;
  u16 tile_x = 0;
  u16 tile_z = 0;
};


// Example :
// overworld::MapGraft::Attach(MapId::kLittlerootTown, overworld::Facing::kUp,
//                           MapId::kOldaleTown);
// overworld::MapGraft::Link(MapId::kOldaleTown, overworld::Facing::kUp,
//                         MapId::kRustboroCity, 125, 238);
// overworld::MapGraft::Link(MapId::kRustboroCity, overworld::Facing::kDown,
//                         MapId::kOldaleTown, 100, 121);
// overworld::MapGraft::Link(MapId::kRustboroCity, overworld::Facing::kRight,
//                         MapId::kPetalburgCity, 80, 126);
// overworld::MapGraft::Link(MapId::kPetalburgCity, overworld::Facing::kLeft,
//                         MapId::kRustboroCity, 159, 173);
// overworld::MapGraft::Link(MapId::kPetalburgCity, overworld::Facing::kRight,
//                         MapId::kFallarborTown, 160, 102);
// overworld::MapGraft::Link(MapId::kFallarborTown, overworld::Facing::kLeft,
//                         MapId::kVerdanturfTown, 119, 102);

class MapGraft {
  MAKE_SINGLETON(MapGraft)

public:
  static constexpr u32 kMaxRequests = 8;
  static constexpr u32 kMaxLinks = 8;

  struct Result {
    bool is_applied = false;
    u16 x = 0;
    u16 z = 0;
    u16 width = 0;
    u16 height = 0;
    s32 offset_x = 0;
    s32 offset_z = 0;
  };

  bool is_enabled = true;
  bool is_logging_enabled = false;

  static void Initialize();
  /// Glues `map` onto the `side` (kUp/kDown/kLeft/kRight) of `anchor`.
  static bool Attach(MapId anchor, Facing side, MapId map, s32 shift = 0);
  static void Clear();
  static u32 GetCount();
  static MapGraftRequest& GetRequest(u32 index);
  static Result& GetResult(u32 index);
  static bool Link(MapId from, Facing side, MapId to, u16 tile_x,
                   u16 tile_z);
  static u32 GetLinkCount();
  static MapLinkRequest& GetLink(u32 index);
  static void Update();
  static bool GetTileOffset(MapId map, s32& dx, s32& dz);
  static void OffsetPlacement(CharacterPlacement* placement, s32 dx, s32 dz);
  static void OffsetEvents(MapEventData* events, s32 dx, s32 dz);
  static void OffsetSettings(MapSettings& settings, s32 dx, s32 dz);

private:
  struct File;
  struct Pristine;
  struct BlockRect;

  static MapSettings* GetSettingsTable();
  void WarpThrough(u32 index);
  void UpdateLinks();
  void TryLinks(MapId from, Facing side, MapId into);
  static bool IsOffTheMap(WorldLayout& layout, s32 tile_x, s32 tile_z);
  void UpdateProps();
  static void LoadWorldLayout(WorldLayout* layout, Bundle* pack);
  static Facing GetStepSide(s32 step_x, s32 step_z);
  static void Warp(const MapLinkRequest& link);
  static void GetSideDelta(u8 rotation, Facing side, s32& dx, s32& dz);
  static bool FindMap(const Pristine& pristine, MapId map, BlockRect* out);
  bool FindAnchor(u32 index, const Pristine& pristine, BlockRect* out) const;
  static void Grow(WorldLayout& layout, const Pristine& pristine, u32 width, u32 height);
  void Apply(WorldLayout& layout, Bundle* pack);
  bool IsBlockOnCopy(u32 x, u32 z) const;
  void SyncSettingsTable(u32 i, s32 dx, s32 dz);
  bool IsCellOnCopy(u32 x, u32 z, MapId map) const;

  static constexpr s32 kTilesPerBlock = 32;
  static constexpr u32 kMaxProps = 256;
  static constexpr u32 kPendingFrames = 120;

  MapGraftRequest requests_[kMaxRequests];
  Result results_[kMaxRequests];
  s32 table_offset_x_[kMaxRequests] = {};
  s32 table_offset_z_[kMaxRequests] = {};
  u32 request_count_ = 0;
  bool has_moved_blocks_ = false;

  MapLinkRequest links_[kMaxLinks];
  u32 link_count_ = 0;
  u32 pending_frames_ = 0;
  bool has_previous_cell_ = false;
  MapId previous_cell_map_ = MapId::kNone;
  s32 previous_tile_x_ = 0;
  s32 previous_tile_z_ = 0;
};

} // namespace overworld
