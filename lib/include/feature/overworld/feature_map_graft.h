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

#ifndef SANGO_PLUGIN_FEATURE_MAP_GRAFT_H
#define SANGO_PLUGIN_FEATURE_MAP_GRAFT_H
#include "common.h"
#include "feature/core/hook_manager.h"
#include "game/constant/map.h"
#include "game/core/manager.h"
#include "game/core/process_manager.h"
#include "game/overworld/character_placement.h"
#include "game/overworld/map_data.h"
#include "game/overworld/map_manager.h"
#include "game/overworld/model_manager.h"
#include "game/overworld/prop_model_manager.h"
#include "game/overworld/world_layout.h"
#include "game/renderer/h3d_shader_model.h"
#include "ui/log_application.h"

namespace feature {

enum class Side : u8 {
  kUp = 0,
  kDown = 1,
  kLeft = 2,
  kRight = 3,
};

struct MapGraftRequest {
  MapId anchor = MapId::kNone;
  Side side = Side::kUp;
  MapId map = MapId::kNone;
  s32 shift = 0;
};

struct MapLinkRequest {
  MapId from = MapId::kNone;
  Side side = Side::kUp;
  MapId to = MapId::kNone;
  u16 tile_x = 0;
  u16 tile_z = 0;
};

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

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadWorldLayout,
                            ADDRESS_LOAD_WORLD_LAYOUT, (uptr)LoadWorldLayout);
  }

  static bool Attach(MapId anchor, Side side, MapId map, s32 shift = 0) {
    auto& ctx = GetInstance();
    if (ctx.request_count_ >= kMaxRequests) return false;
    MapGraftRequest& request = ctx.requests_[ctx.request_count_++];
    request.anchor = anchor;
    request.side = side;
    request.map = map;
    request.shift = shift;
    return true;
  }

  static void Clear() {
    auto& ctx = GetInstance();
    for (u32 i = 0; i < ctx.request_count_; i++) ctx.SyncSettingsTable(i, 0, 0);
    ctx.request_count_ = 0;
    ctx.link_count_ = 0;
  }
  static u32 GetCount() { return GetInstance().request_count_; }

  static MapGraftRequest& GetRequest(u32 index) {
    return GetInstance().requests_[index < kMaxRequests ? index : 0];
  }

  static Result& GetResult(u32 index) {
    return GetInstance().results_[index < kMaxRequests ? index : 0];
  }

  static bool Link(MapId from, Side side, MapId to, u16 tile_x, u16 tile_z) {
    auto& ctx = GetInstance();
    if (ctx.link_count_ >= kMaxLinks) return false;
    MapLinkRequest& link = ctx.links_[ctx.link_count_++];
    link.from = from;
    link.side = side;
    link.to = to;
    link.tile_x = tile_x;
    link.tile_z = tile_z;
    return true;
  }

  static u32 GetLinkCount() { return GetInstance().link_count_; }

  static MapLinkRequest& GetLink(u32 index) {
    return GetInstance().links_[index < kMaxLinks ? index : 0];
  }

  static void Update() {
    auto& ctx = GetInstance();
    if (!game::ProcessManager::IsOverworldActive()) {
      ctx.has_previous_cell_ = false;
      return;
    }
    ctx.UpdateProps();
    ctx.UpdateLinks();
  }

  static bool GetTileOffset(MapId map, s32& dx, s32& dz) {
    auto& ctx = GetInstance();
    for (u32 i = 0; i < ctx.request_count_; i++) {
      if (ctx.requests_[i].map != map || !ctx.results_[i].is_applied) continue;
      const s32 tiles = overworld::WorldLayout::kTilesPerBlockSide;
      dx = ctx.results_[i].offset_x * tiles;
      dz = ctx.results_[i].offset_z * tiles;
      return true;
    }
    return false;
  }

  static void OffsetPlacement(overworld::CharacterPlacement* placement,
                              s32 dx, s32 dz) {
    if (placement->position.kind == overworld::PositionKind::kTileGrid) {
      placement->position.tile_x += dx;
      placement->position.tile_z += dz;
    }
    placement->trigger_x += dx;
    placement->trigger_z += dz;
  }

  static void OffsetEvents(uptr events, s32 dx, s32 dz) {
    using namespace overworld;
    const u32 grid = static_cast<u32>(PositionKind::kTileGrid);

    auto* signs = *(SignEvent**)(events + map_event_offsets::kSigns);
    const u32 sign_count = READ16(events + map_event_offsets::kSignCount);
    for (u32 i = 0; signs != nullptr && i < sign_count; i++) {
      if (signs[i].position_kind != grid) continue;
      signs[i].tiles.tile_x += dx;
      signs[i].tiles.tile_z += dz;
    }

    auto* warps = *(WarpEvent**)(events + map_event_offsets::kWarps);
    const u32 warp_count = READ16(events + map_event_offsets::kWarpCount);
    for (u32 i = 0; warps != nullptr && i < warp_count; i++) {
      if (warps[i].position_kind != grid) continue;
      warps[i].world.x += dx * static_cast<s32>(WorldLayout::kUnitsPerTile);
      warps[i].world.z += dz * static_cast<s32>(WorldLayout::kUnitsPerTile);
    }

    const u32 tables[2][2] = {
        {map_event_offsets::kTriggers, map_event_offsets::kTriggerCount},
        {map_event_offsets::kInterrupts, map_event_offsets::kInterruptCount},
    };
    for (u32 t = 0; t < 2; t++) {
      auto* triggers = *(TriggerEvent**)(events + tables[t][0]);
      const u32 count = READ16(events + tables[t][1]);
      for (u32 i = 0; triggers != nullptr && i < count; i++) {
        if (triggers[i].position_kind != grid) continue;
        triggers[i].tiles.tile_x += dx;
        triggers[i].tiles.tile_z += dz;
      }
    }
  }

  static void OffsetSettings(overworld::MapSettings& settings, s32 dx,
                             s32 dz) {
    const s32 ux = dx * static_cast<s32>(overworld::WorldLayout::kUnitsPerTile);
    const s32 uz = dz * static_cast<s32>(overworld::WorldLayout::kUnitsPerTile);
    settings.fly_x += ux;
    settings.fly_z += uz;
    settings.start_x += ux;
    settings.start_z += uz;
  }

private:
  static overworld::MapSettings* GetSettingsTable() {
    return *(overworld::MapSettings**)ADDRESS_MAP_SETTINGS_TABLE;
  }

  void WarpThrough(u32 index) {
    const MapLinkRequest& link = links_[index];
    if (is_logging_enabled) {
      ui::LogApplication::Print(u"link %u: to map %u at tile %u,%u", index,
                                static_cast<u32>(link.to), link.tile_x,
                                link.tile_z);
    }
    pending_frames_ = kPendingFrames;
    Warp(link);
  }

  struct File {
    u16 has_map_table;
    u16 facing;
    u16 width;
    u16 height;
    u16 table[];
  };

  struct Pristine {
    const File* file;
    MapId loaded_map_id;

    INLINE u32 GetCellWidth() const {
      return file->width * overworld::WorldLayout::kCellsPerBlockSide;
    }
    INLINE u32 GetCellHeight() const {
      return file->height * overworld::WorldLayout::kCellsPerBlockSide;
    }
    INLINE u16 BlockAt(u32 x, u32 z) const {
      return file->table[x + z * file->width];
    }
    INLINE MapId MapAt(u32 cell_x, u32 cell_z) const {
      if (!file->has_map_table) return loaded_map_id;
      const u16* cells = file->table + file->width * file->height;
      return static_cast<MapId>(cells[cell_x + cell_z * GetCellWidth()]);
    }
    INLINE u16 LodAt(u32 x, u32 z) const {
      if (!file->has_map_table) return overworld::WorldLayout::kNoLod;
      const u32 blocks = file->width * file->height;
      const u16* lods = file->table + blocks + GetCellWidth() * GetCellHeight();
      return lods[x + z * file->width];
    }
  };

  struct BlockRect {
    s32 x0 = 0;
    s32 z0 = 0;
    s32 x1 = -1;
    s32 z1 = -1;

    INLINE s32 GetWidth() const { return x1 - x0 + 1; }
    INLINE s32 GetHeight() const { return z1 - z0 + 1; }
  };

  void UpdateLinks() {
    using overworld::WorldLayout;
    if (link_count_ == 0 || !is_enabled) return;
    if (pending_frames_ > 0) {
      pending_frames_--;
      return;
    }

    auto& player = overworld::ModelManager::GetInstance().GetPlayer();
    if (player.id != overworld::ModelManager::kPlayerId) return;
    auto& layout = WorldLayout::GetInstance();
    const s32 tile_x = static_cast<s32>(player.map_pos.coords.x);
    const s32 tile_z = static_cast<s32>(player.map_pos.coords.z);
    const u32 cell_x = tile_x / WorldLayout::kTilesPerCellSide;
    const u32 cell_z = tile_z / WorldLayout::kTilesPerCellSide;
    if (tile_x < 0 || tile_z < 0 || !layout.ContainsCell(cell_x, cell_z)) {
      has_previous_cell_ = false;
      return;
    }
    const MapId cell_map = layout.MapAt(cell_x, cell_z);

    if (!has_previous_cell_) {
      has_previous_cell_ = true;
      previous_cell_map_ = cell_map;
      previous_tile_x_ = tile_x;
      previous_tile_z_ = tile_z;
      return;
    }
    const MapId from = previous_cell_map_;
    const s32 step_x = tile_x - previous_tile_x_;
    const s32 step_z = tile_z - previous_tile_z_;
    previous_tile_x_ = tile_x;
    previous_tile_z_ = tile_z;

    if (cell_map == from) {
      const Side side = GetStepSide(0, 0);
      s32 dx, dz;
      GetSideDelta(layout.facing, side, dx, dz);
      if (IsOffTheMap(layout, tile_x + dx, tile_z + dz)) {
        TryLinks(from, side, MapId::kNone);
      }
      return;
    }
    if (cell_map != MapId::kNone) previous_cell_map_ = cell_map;

    const Side side = GetStepSide(step_x, step_z);
    if (is_logging_enabled) {
      ui::LogApplication::Print(u"crossed map %u -> map %u through side %u",
                                static_cast<u32>(from),
                                static_cast<u32>(cell_map),
                                static_cast<u32>(side));
    }
    TryLinks(from, side, cell_map);
  }

  void TryLinks(MapId from, Side side, MapId into) {
    for (u32 i = 0; i < link_count_; i++) {
      const MapLinkRequest& link = links_[i];
      if (link.from != from || link.side != side) continue;
      if (link.to == into) continue;
      WarpThrough(i);
      return;
    }
  }

  static bool IsOffTheMap(overworld::WorldLayout& layout, s32 tile_x,
                          s32 tile_z) {
    using overworld::WorldLayout;
    if (tile_x < 0 || tile_z < 0) return true;
    const u32 cell_x = tile_x / WorldLayout::kTilesPerCellSide;
    const u32 cell_z = tile_z / WorldLayout::kTilesPerCellSide;
    if (!layout.ContainsCell(cell_x, cell_z)) return true;
    if (layout.MapAt(cell_x, cell_z) == MapId::kNone) return true;
    const u32 block_x = tile_x / WorldLayout::kTilesPerBlockSide;
    const u32 block_z = tile_z / WorldLayout::kTilesPerBlockSide;
    return layout.BlockAt(block_x, block_z) == WorldLayout::kNoBlock;
  }

  void UpdateProps() {
    using overworld::WorldLayout;
    if (!has_moved_blocks_) return;
    auto* field = &game::Manager::GetInstance().GetOverworldMapManager();
    if (field == nullptr) return;
    auto* props = &field->GetPropModelManager();
    if (props == nullptr || props->prop_models == nullptr) return;
    auto& layout = WorldLayout::GetInstance();
    const f32 block = static_cast<f32>(kTilesPerBlock *
                                       WorldLayout::kUnitsPerTile);

    const u32 capacity = props->capacity < kMaxProps ? props->capacity
                                                     : kMaxProps;
    for (u32 i = 0; i < capacity; i++) {
      overworld::PropModel& prop = props->prop_models[i];
      if (!prop.is_initialized || prop.shader == nullptr) continue;
      Vec3 position = prop.shader->position;
      if (position.x < 0.0f || position.z < 0.0f) continue;
      const s32 block_x = static_cast<s32>(position.x / block);
      const s32 block_z = static_cast<s32>(position.z / block);

      for (u32 r = 0; r < request_count_; r++) {
        const Result& result = results_[r];
        if (!result.is_applied) continue;
        const s32 source_x = result.x - result.offset_x;
        const s32 source_z = result.z - result.offset_z;
        if (block_x < source_x || block_x >= source_x + result.width) continue;
        if (block_z < source_z || block_z >= source_z + result.height) continue;
        const u32 dest_x = block_x + result.offset_x;
        const u32 dest_z = block_z + result.offset_z;
        if (!layout.ContainsBlock(dest_x, dest_z)) continue;
        if (layout.BlockAt(dest_x, dest_z) != prop.map_index) continue;
        position.x += result.offset_x * block;
        position.z += result.offset_z * block;
        prop.shader->SetTranslate(position);
        prop.needs_bounds_update = true;
        break;
      }
    }
  }

  static void LoadWorldLayout(overworld::WorldLayout* layout, Bundle* pack) {
    HookManager::Call<void>(HookID::kLoadWorldLayout, layout, pack);
    GetInstance().pending_frames_ = 0;
    GetInstance().has_previous_cell_ = false;
    GetInstance().Apply(*layout, pack);
  }

  static Side GetStepSide(s32 step_x, s32 step_z) {
    f32 way_x = step_x;
    f32 way_z = step_z;
    if (step_x == 0 && step_z == 0) {
      const Vec3& facing = overworld::ModelManager::GetInstance().GetPlayer().
                           facing_direction;
      way_x = facing.x;
      way_z = facing.z;
    }
    const f32 abs_x = way_x < 0.0f ? -way_x : way_x;
    const f32 abs_z = way_z < 0.0f ? -way_z : way_z;
    const s32 grid_x = abs_x > abs_z ? (way_x < 0.0f ? -1 : 1) : 0;
    const s32 grid_z = abs_x > abs_z ? 0 : (way_z < 0.0f ? -1 : 1);

    const u8 rotation = overworld::WorldLayout::GetInstance().facing;
    for (u8 side = 0; side < 4; side++) {
      s32 dx, dz;
      GetSideDelta(rotation, static_cast<Side>(side), dx, dz);
      if (dx == grid_x && dz == grid_z) return static_cast<Side>(side);
    }
    return Side::kUp;
  }

  static void Warp(const MapLinkRequest& link) {
    using overworld::WorldLayout;
    const f32 unit = WorldLayout::kUnitsPerTile;
    overworld::Position position = overworld::ModelManager::GetInstance().
                                   GetPlayer().world_pos;
    position.coords.x = link.tile_x * unit + unit / 2.0f;
    position.coords.z = link.tile_z * unit + unit / 2.0f;

    const u8 facing = static_cast<u8>(link.side);
    const bool keep_music = false;
    const bool show_map_name = true;
    ((void (*)(game::Manager*, u16, const overworld::Position*, u8, u8, bool,
               s32, s32, s32, bool))ADDRESS_CHANGE_MAP)(
        &game::Manager::GetInstance(), static_cast<u16>(link.to), &position,
        facing, 0, keep_music, 1, 1, 1, show_map_name);
  }

  static void GetSideDelta(u8 facing, Side side, s32& dx, s32& dz) {
    static const s8 kDelta[4][4][2] = {
        {{0, -1}, {0, 1}, {-1, 0}, {1, 0}},
        {{0, 1}, {0, -1}, {1, 0}, {-1, 0}},
        {{-1, 0}, {1, 0}, {0, 1}, {0, -1}},
        {{1, 0}, {-1, 0}, {0, -1}, {0, 1}},
    };
    const u8 rotation = facing < 4 ? facing : 0;
    dx = kDelta[rotation][static_cast<u8>(side)][0];
    dz = kDelta[rotation][static_cast<u8>(side)][1];
  }

  static bool FindMap(const Pristine& pristine, MapId map, BlockRect* out) {
    const u32 cell_width = pristine.GetCellWidth();
    const u32 cell_height = pristine.GetCellHeight();
    s32 x0 = cell_width, z0 = cell_height, x1 = -1, z1 = -1;
    for (u32 z = 0; z < cell_height; z++) {
      for (u32 x = 0; x < cell_width; x++) {
        if (pristine.MapAt(x, z) != map) continue;
        if ((s32)x < x0) x0 = x;
        if ((s32)x > x1) x1 = x;
        if ((s32)z < z0) z0 = z;
        if ((s32)z > z1) z1 = z;
      }
    }
    if (x1 < 0) return false;

    const s32 cells = overworld::WorldLayout::kCellsPerBlockSide;
    out->x0 = x0 / cells;
    out->z0 = z0 / cells;
    out->x1 = x1 / cells;
    out->z1 = z1 / cells;
    return true;
  }

  bool FindAnchor(u32 index, const Pristine& pristine, BlockRect* out) const {
    const MapId anchor = requests_[index].anchor;
    for (u32 i = 0; i < index; i++) {
      if (requests_[i].map != anchor || !results_[i].is_applied) continue;
      out->x0 = results_[i].x;
      out->z0 = results_[i].z;
      out->x1 = results_[i].x + results_[i].width - 1;
      out->z1 = results_[i].z + results_[i].height - 1;
      return true;
    }
    return FindMap(pristine, anchor, out);
  }

  static void Grow(overworld::WorldLayout& layout, const Pristine& pristine,
                   u32 width, u32 height) {
    using overworld::WorldLayout;
    const u32 old_width = pristine.file->width;
    const u32 old_height = pristine.file->height;
    layout.width = width;
    layout.height = height;

    for (u32 z = 0; z < height; z++) {
      for (u32 x = 0; x < width; x++) {
        const bool exists = x < old_width && z < old_height;
        layout.BlockAt(x, z) = exists ? pristine.BlockAt(x, z)
                                      : WorldLayout::kNoBlock;
        layout.LodAt(x, z) = exists ? pristine.LodAt(x, z)
                                    : WorldLayout::kNoLod;
      }
    }

    const u32 old_cell_width = pristine.GetCellWidth();
    const u32 old_cell_height = pristine.GetCellHeight();
    for (u32 z = 0; z < layout.GetCellHeight(); z++) {
      for (u32 x = 0; x < layout.GetCellWidth(); x++) {
        const bool exists = x < old_cell_width && z < old_cell_height;
        layout.MapAt(x, z) = exists ? pristine.MapAt(x, z) : MapId::kNone;
      }
    }
  }

  void Apply(overworld::WorldLayout& layout, Bundle* pack) {
    using overworld::WorldLayout;

    for (u32 i = 0; i < kMaxRequests; i++) results_[i] = Result{};
    has_moved_blocks_ = false;
    if (!is_enabled || request_count_ == 0) return;
    if (pack == nullptr || pack->resource_count == 0) return;

    const Pristine pristine{(const File*)pack->GetResource(0),
                            layout.loaded_map_id};
    if (pristine.file->width != layout.width ||
        pristine.file->height != layout.height) {
      return;
    }

    u32 width = layout.width;
    u32 height = layout.height;
    for (u32 i = 0; i < request_count_; i++) {
      const MapGraftRequest& request = requests_[i];
      BlockRect anchor;
      if (!FindAnchor(i, pristine, &anchor)) continue;
      BlockRect source;
      if (!FindMap(pristine, request.map, &source)) {
        if (is_logging_enabled) {
          ui::LogApplication::Print(
              u"graft %u: map %u is not on layout %u, only Link() can join it",
              i, static_cast<u32>(request.map), layout.id);
        }
        continue;
      }

      s32 side_x, side_z;
      GetSideDelta(layout.facing, request.side, side_x, side_z);
      s32 seam_x, seam_z;
      GetSideDelta(layout.facing, side_z != 0 ? Side::kRight : Side::kDown,
                   seam_x, seam_z);

      BlockRect dest;
      if (side_z < 0) {
        dest.z1 = anchor.z0 - 1;
        dest.z0 = dest.z1 - source.GetHeight() + 1;
        dest.x0 = anchor.x0;
        dest.x1 = dest.x0 + source.GetWidth() - 1;
      } else if (side_z > 0) {
        dest.z0 = anchor.z1 + 1;
        dest.z1 = dest.z0 + source.GetHeight() - 1;
        dest.x0 = anchor.x0;
        dest.x1 = dest.x0 + source.GetWidth() - 1;
      } else if (side_x < 0) {
        dest.x1 = anchor.x0 - 1;
        dest.x0 = dest.x1 - source.GetWidth() + 1;
        dest.z0 = anchor.z0;
        dest.z1 = dest.z0 + source.GetHeight() - 1;
      } else {
        dest.x0 = anchor.x1 + 1;
        dest.x1 = dest.x0 + source.GetWidth() - 1;
        dest.z0 = anchor.z0;
        dest.z1 = dest.z0 + source.GetHeight() - 1;
      }
      dest.x0 += request.shift * seam_x;
      dest.x1 += request.shift * seam_x;
      dest.z0 += request.shift * seam_z;
      dest.z1 += request.shift * seam_z;

      if (dest.x0 < 0 || dest.z0 < 0 ||
          dest.x1 >= (s32)WorldLayout::kMaxWidth ||
          dest.z1 >= (s32)WorldLayout::kMaxHeight) {
        if (is_logging_enabled) {
          ui::LogApplication::Print(u"graft %u: map %u falls off the grid",
                                    i, static_cast<u32>(request.map));
        }
        continue;
      }
      if ((u32)dest.x1 >= width) width = dest.x1 + 1;
      if ((u32)dest.z1 >= height) height = dest.z1 + 1;

      Result& result = results_[i];
      result.is_applied = true;
      result.x = dest.x0;
      result.z = dest.z0;
      result.width = source.GetWidth();
      result.height = source.GetHeight();
      result.offset_x = dest.x0 - source.x0;
      result.offset_z = dest.z0 - source.z0;
    }

    if (width != layout.width || height != layout.height) {
      Grow(layout, pristine, width, height);
    }

    const u32 cells = WorldLayout::kCellsPerBlockSide;
    for (u32 i = 0; i < request_count_; i++) {
      const Result& result = results_[i];
      if (!result.is_applied) continue;
      const MapId map = requests_[i].map;
      const u32 source_x = result.x - result.offset_x;
      const u32 source_z = result.z - result.offset_z;

      for (u32 z = 0; z < result.height; z++) {
        for (u32 x = 0; x < result.width; x++) {
          layout.BlockAt(result.x + x, result.z + z) =
              pristine.BlockAt(source_x + x, source_z + z);
          layout.LodAt(result.x + x, result.z + z) =
              pristine.LodAt(source_x + x, source_z + z);
        }
      }
      for (u32 z = 0; z < result.height * cells; z++) {
        for (u32 x = 0; x < result.width * cells; x++) {
          const MapId owner = pristine.MapAt(source_x * cells + x,
                                             source_z * cells + z);
          layout.MapAt(result.x * cells + x, result.z * cells + z) =
              owner == MapId::kNone ? MapId::kNone : map;
        }
      }

      if (is_logging_enabled) {
        ui::LogApplication::Print(
            u"graft %u: map %u copied to block %u,%u (%ux%u) offset %d,%d",
            i, static_cast<u32>(map), result.x, result.z, result.width,
            result.height, result.offset_x, result.offset_z);
      }
    }

    for (u32 i = 0; i < request_count_; i++) {
      const Result& result = results_[i];
      if (!result.is_applied) continue;
      has_moved_blocks_ = true;
      const MapId map = requests_[i].map;
      for (u32 z = 0; z < layout.GetCellHeight(); z++) {
        for (u32 x = 0; x < layout.GetCellWidth(); x++) {
          if (layout.MapAt(x, z) != map) continue;
          if (IsCellOnCopy(x, z, map)) continue;
          layout.MapAt(x, z) = MapId::kNone;
        }
      }
      const u32 source_x = result.x - result.offset_x;
      const u32 source_z = result.z - result.offset_z;
      for (u32 z = 0; z < result.height; z++) {
        for (u32 x = 0; x < result.width; x++) {
          if (IsBlockOnCopy(source_x + x, source_z + z)) continue;
          layout.BlockAt(source_x + x, source_z + z) = WorldLayout::kNoBlock;
          layout.LodAt(source_x + x, source_z + z) = WorldLayout::kNoLod;
        }
      }
      SyncSettingsTable(i, result.offset_x * kTilesPerBlock,
                        result.offset_z * kTilesPerBlock);
    }
  }

  bool IsBlockOnCopy(u32 x, u32 z) const {
    for (u32 i = 0; i < request_count_; i++) {
      const Result& result = results_[i];
      if (!result.is_applied) continue;
      if (x < result.x || x >= result.x + result.width) continue;
      if (z < result.z || z >= result.z + result.height) continue;
      return true;
    }
    return false;
  }

  void SyncSettingsTable(u32 i, s32 dx, s32 dz) {
    const u32 map = static_cast<u32>(requests_[i].map);
    if (map >= MAP_SETTINGS_TABLE_COUNT) return;
    if (table_offset_x_[i] == dx && table_offset_z_[i] == dz) return;
    overworld::MapSettings* table = GetSettingsTable();
    if (table == nullptr) return;
    OffsetSettings(table[map], dx - table_offset_x_[i],
                   dz - table_offset_z_[i]);
    table_offset_x_[i] = dx;
    table_offset_z_[i] = dz;
  }

  bool IsCellOnCopy(u32 x, u32 z, MapId map) const {
    const u32 cells = overworld::WorldLayout::kCellsPerBlockSide;
    for (u32 i = 0; i < request_count_; i++) {
      const Result& result = results_[i];
      if (!result.is_applied || requests_[i].map != map) continue;
      if (x < result.x * cells || x >= (result.x + result.width) * cells) {
        continue;
      }
      if (z < result.z * cells || z >= (result.z + result.height) * cells) {
        continue;
      }
      return true;
    }
    return false;
  }

  static constexpr s32 kTilesPerBlock =
      overworld::WorldLayout::kTilesPerBlockSide;

  MapGraftRequest requests_[kMaxRequests];
  Result results_[kMaxRequests];
  s32 table_offset_x_[kMaxRequests] = {};
  s32 table_offset_z_[kMaxRequests] = {};
  u32 request_count_ = 0;
  bool has_moved_blocks_ = false;
  static constexpr u32 kMaxProps = 256;

  static constexpr u32 kPendingFrames = 120;

  MapLinkRequest links_[kMaxLinks];
  u32 link_count_ = 0;
  u32 pending_frames_ = 0;
  bool has_previous_cell_ = false;
  MapId previous_cell_map_ = MapId::kNone;
  s32 previous_tile_x_ = 0;
  s32 previous_tile_z_ = 0;
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_MAP_GRAFT_H
