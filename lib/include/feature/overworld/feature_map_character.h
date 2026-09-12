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

#ifndef SANGO_PLUGIN_FEATURE_MAP_CHARACTER_H
#define SANGO_PLUGIN_FEATURE_MAP_CHARACTER_H
#include "common.h"
#include "feature/core/hook_manager.h"
#include "feature/overworld/feature_map_graft.h"
#include "game/constant/model.h"
#include "game/constant/script.h"
#include "game/core/manager.h"
#include "game/core/process_manager.h"
#include "game/overworld/character_placement.h"
#include "game/overworld/map_manager.h"
#include "game/overworld/model_manager.h"
#include "ui/log_application.h"

namespace ui {
class LogApplication;
}

namespace feature {

struct MapCharacterRequest {
  u16 map_id = 0;
  ModelId model_id = ModelId::kNone;
  ScriptId script_id = ScriptId::kNone;
  u16 tile_x = 0;
  u16 tile_z = 0;
  f32 height = 0.0f;
  overworld::Facing facing = overworld::Facing::kDown;
  u16 movement_id = 0;
  u16 hide_when_flag_set = 0;
};

class MapCharacter {
  MAKE_SINGLETON(MapCharacter)

public:
  static constexpr u32 kMaxRequests = 16;
  bool is_logging_enabled = false;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadMapCharacters,
                            ADDRESS_OVERWORLD_LOAD_MAP_CHARACTERS,
                            (uptr)LoadMapCharacters);
    HookManager::Initialize(HookID::kCompleteRegionModelList,
                            ADDRESS_OVERWORLD_COMPLETE_REGION_MODEL_LIST,
                            (uptr)CompleteRegionModelList);
  }

  static bool Add(const MapCharacterRequest& request) {
    auto& ctx = GetInstance();
    if (ctx.request_count_ >= kMaxRequests) return false;
    ctx.requests_[ctx.request_count_++] = request;
    return true;
  }

  static void Clear() { GetInstance().request_count_ = 0; }
  static u32 GetCount() { return GetInstance().request_count_; }

  static void ReloadMapAfterBattleWith(u16 trainer_id) {
    auto& ctx = GetInstance();
    for (u32 i = 0; i < ctx.request_count_; i++) {
      const MapCharacterRequest& request = ctx.requests_[i];
      if (request.hide_when_flag_set == 0) continue;
      if (static_cast<u16>(request.script_id) != kFirstTrainerScript +
          trainer_id) {
        continue;
      }
      ctx.is_reload_armed_ = true;
      ctx.has_resting_spot_ = false;
      return;
    }
  }

  static void Update() {
    auto& ctx = GetInstance();
    if (!ctx.is_reload_armed_) return;

    if (!game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_OVERWORLD_VTABLE)) {
      ctx.has_resting_spot_ = false;
      return;
    }

    const Vec3& stood_at = overworld::ModelManager::GetInstance().GetPlayer().
                           map_pos.coords;
    if (!ctx.has_resting_spot_) {
      ctx.resting_x_ = stood_at.x;
      ctx.resting_z_ = stood_at.z;
      ctx.has_resting_spot_ = true;
      return;
    }
    if (stood_at.x == ctx.resting_x_ && stood_at.z == ctx.resting_z_) return;

    ctx.is_reload_armed_ = false;
    ctx.has_resting_spot_ = false;
    ReloadCurrentMap();
  }

private:
  static constexpr u16 kFirstTrainerScript = 3000;

  static void ReloadCurrentMap() {
    overworld::Position position = overworld::ModelManager::GetInstance().
                                   GetPlayer().world_pos;
    u16 map_id = (u16)overworld::MapManager::GetInstance().GetMapId();

    ((void (*)(game::Manager*, u16, const overworld::Position*, u8, u8, bool,
               s32, s32, s32, bool))ADDRESS_CHANGE_MAP)(
        &game::Manager::GetInstance(), map_id, &position, 0, 0, true, 1, 1, 1,
        false);
  }

  static u32 LoadMapCharacters(uptr event_data, u32 buffer_id) {
    u32 result = HookManager::Call<u32>(HookID::kLoadMapCharacters, event_data,
                                        buffer_id);
    auto& ctx = GetInstance();
    if (ctx.is_logging_enabled) ctx.LogShippedCharacters(event_data);
    ctx.MoveGraftedEvents(event_data);
    ctx.PlaceCharacters(event_data);
    return result;
  }

  static void MoveGraftedEvents(uptr event_data) {
    using namespace overworld;
    const u16 map_id = READ16(event_data + map_event_offsets::kMapId);
    s32 dx = 0;
    s32 dz = 0;
    if (!MapGraft::GetTileOffset(static_cast<MapId>(map_id), dx, dz)) return;
    MapGraft::OffsetEvents(event_data, dx, dz);
  }

  static void CompleteRegionModelList(uptr manager, u32 player_sex,
                                      uptr placements, u32 placement_count) {
    HookManager::Call<void>(HookID::kCompleteRegionModelList, manager,
                            player_sex, placements, placement_count);
    GetInstance().AddMissingModels(
        manager, (const overworld::CharacterPlacement*)placements,
        placement_count);
  }

  void LogShippedCharacters(uptr event_data) const {
    using namespace overworld;

    u16 map_id = READ16(event_data + map_event_offsets::kMapId);
    u32 count = READ16(event_data + map_event_offsets::kCharacterCount);
    auto* shipped = *(CharacterPlacement**)(event_data +
                                            map_event_offsets::kCharacters);
    ui::LogApplication::Print(u"map=%u chars=%u", map_id, count);
    if (shipped == nullptr) return;

    u32 shown = count < 8 ? count : 8;
    for (u32 i = 0; i < shown; i++) {
      const CharacterPlacement& c = shipped[i];
      ui::LogApplication::Print(u"  id=%u model=%u script=%u xz=%u,%u y=%d",
                                c.local_id, c.model_id, c.script_id,
                                c.position.tile_x, c.position.tile_z,
                                (s32)c.position.height);
    }
  }

  void PlaceCharacters(uptr event_data) {
    using namespace overworld;

    u16 map_id = READ16(event_data + map_event_offsets::kMapId);

    auto* shipped = *(CharacterPlacement**)(event_data +
                                            map_event_offsets::kCharacters);
    u32 shipped_count = READ16(event_data +
                               map_event_offsets::kCharacterCount);
    if (shipped == nullptr) return;
    if (shipped_count > kMaxCharactersPerMap) return;

    s32 graft_dx = 0;
    s32 graft_dz = 0;
    const bool is_grafted = MapGraft::GetTileOffset(
        static_cast<MapId>(map_id), graft_dx, graft_dz);

    u32 count = 0;
    u16 next_local_id = 0;
    for (; count < shipped_count; count++) {
      placements_[count] = shipped[count];
      if (is_grafted) {
        MapGraft::OffsetPlacement(&placements_[count], graft_dx, graft_dz);
      }
      if (shipped[count].local_id >= next_local_id) {
        next_local_id = shipped[count].local_id + 1;
      }
    }

    u32 added = 0;
    for (u32 i = 0; i < request_count_ && count < kMaxCharactersPerMap; i++) {
      const MapCharacterRequest& request = requests_[i];
      if (request.map_id != map_id) continue;
      BuildPlacement(request, next_local_id++, &placements_[count++]);
      added++;
    }

    if (added == 0 && !is_grafted) return;

    WRITE32(event_data + map_event_offsets::kCharacters, (u32)placements_);
    WRITE16(event_data + map_event_offsets::kCharacterCount, count);
    WRITE16(event_data + map_event_offsets::kCharacterCapacity, count);

    if (is_logging_enabled) {
      ui::LogApplication::Print(u"placed %u -> chars=%u", added, count);
    }
  }

  static void BuildPlacement(const MapCharacterRequest& request, u16 local_id,
                             overworld::CharacterPlacement* out) {
    *out = overworld::CharacterPlacement{};
    out->local_id = local_id;
    out->model_id = request.model_id;
    out->movement_id = request.movement_id;
    out->interaction_kind = 0;
    out->hide_when_flag_set = request.hide_when_flag_set;
    out->script_id = request.script_id;
    out->facing = (u16)request.facing;
    out->displayed_on_map_id = request.map_id;
    out->owned_by_map_id = request.map_id;
    out->map_id = request.map_id;
    out->position.kind = overworld::PositionKind::kTileGrid;
    out->position.tile_x = request.tile_x;
    out->position.tile_z = request.tile_z;
    out->position.height = request.height;
  }

  void AddMissingModels(uptr manager,
                        const overworld::CharacterPlacement* placements,
                        u32 placement_count) {
    using namespace overworld;
    if (placements == nullptr) return;

    auto* models = (ModelAppearance*)(manager +
                                      character_manager_offsets::kModelList);
    u32 count = READ32(manager + character_manager_offsets::kModelCount);
    u32 initial_count = count;

    for (u32 i = 0; i < placement_count; i++) {
      if (count + 1 >= kMaxModelsPerRegion) break;
      ModelId model_id = placements[i].model_id;
      if (Contains(models, count, model_id)) continue;
      if (!LoadAppearance(manager, model_id, &models[count])) {
        if (is_logging_enabled) {
          ui::LogApplication::Print(u"model %u unknown", model_id);
        }
        continue;
      }
      count++;
      if (is_logging_enabled) {
        ui::LogApplication::Print(u"model %u added -> models=%u", model_id,
                                  count);
      }
    }

    if (count == initial_count) return;
    WRITE32(manager + character_manager_offsets::kModelCount, count);
    WRITE32(manager + character_manager_offsets::kModelCountOriginal, count);
  }

  static bool Contains(const overworld::ModelAppearance* models, u32 count,
                       ModelId model_id) {
    for (u32 i = 0; i < count; i++) {
      if (models[i].model_id == model_id) return true;
    }
    return false;
  }

  static bool LoadAppearance(uptr manager, ModelId model_id,
                             overworld::ModelAppearance* out) {
    s32 index = ((s32 (*)(ModelId))ADDRESS_OVERWORLD_GET_MODEL_ARCHIVE_INDEX)(
        model_id);
    if (index < 0) return false;

    void* archive =
        *(void**)(manager +
                  overworld::character_manager_offsets::kModelParamArchive);
    if (archive == nullptr) return false;

    ((void (*)(void*, s32, void*))ADDRESS_ARCHIVE_LOAD_DATA_2)(archive, index,
                                                               out);
    return true;
  }

  MapCharacterRequest requests_[kMaxRequests];
  u32 request_count_ = 0;
  bool is_reload_armed_ = false;
  bool has_resting_spot_ = false;
  f32 resting_x_ = 0.0f;
  f32 resting_z_ = 0.0f;
  overworld::CharacterPlacement placements_[overworld::kMaxCharactersPerMap];
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_MAP_CHARACTER_H
