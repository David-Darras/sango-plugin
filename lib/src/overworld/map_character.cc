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

#include "overworld/patch/map_character.h"
#include "core/hook_manager.h"
#include "core/native/process_manager.h"
#include "overworld/native/character_placement.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/patch/map_graft.h"
#include "ui/log_application.h"

namespace overworld {

void MapCharacter::Initialize() {
  core::HookManager::Initialize(HookId::kLoadMapCharacters,
                          address::kLoadMapCharacters,
                          (uptr)LoadMapCharacters);
  core::HookManager::Initialize(HookId::kCompleteRegionModelList,
                          address::kCompleteRegionModelList,
                          (uptr)CompleteRegionModelList);
}

bool MapCharacter::Add(const MapCharacterRequest& request) {
  auto& ctx = GetInstance();
  if (ctx.request_count_ >= kMaxRequests) return false;
  ctx.requests_[ctx.request_count_++] = request;
  return true;
}

void MapCharacter::Clear() {
  GetInstance().request_count_ = 0;
  GetInstance().emptied_count_ = 0;
}

bool MapCharacter::Empty(MapId map_id) {
  auto& ctx = GetInstance();
  if (ctx.emptied_count_ >= kMaxEmptiedMaps) return false;
  ctx.emptied_maps_[ctx.emptied_count_++] = map_id;
  return true;
}

u32 MapCharacter::GetCount() {
  return GetInstance().request_count_;
}

void MapCharacter::ReloadMapAfterBattleWith(u16 trainer_id) {
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

void MapCharacter::Update() {
  auto& ctx = GetInstance();
  if (!ctx.is_reload_armed_) return;

  if (!core::ProcessManager::GetInstance().IsCurrentProcess(
      address::kVtable)) {
    ctx.has_resting_spot_ = false;
    return;
  }

  const Vec3& stood_at = ModelManager::GetInstance().GetPlayer().
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

void MapCharacter::ReloadCurrentMap() {
  const Position& position = ModelManager::GetInstance().GetPlayer().world_pos;
  MapManager::ChangeMap(MapManager::GetInstance().GetMap(), position,
                        Facing::kUp, true, false);
}

u32 MapCharacter::LoadMapCharacters(MapEventData* events, u32 buffer_id) {
  u32 result = core::HookManager::Call<u32>(HookId::kLoadMapCharacters,
                                            events, buffer_id);
  auto& ctx = GetInstance();
  if (ctx.is_logging_enabled) ctx.LogShippedCharacters(events);
  ctx.MoveGraftedEvents(events);
  ctx.PlaceCharacters(events);
  return result;
}

void MapCharacter::MoveGraftedEvents(MapEventData* events) {
  s32 dx = 0;
  s32 dz = 0;
  if (!MapGraft::GetTileOffset(events->map_id, dx, dz)) return;
  MapGraft::OffsetEvents(events, dx, dz);
}

void MapCharacter::CompleteRegionModelList(
    CharacterManager* manager, u32 player_sex,
    const CharacterPlacement* placements, u32 placement_count) {
  core::HookManager::Call<void>(HookId::kCompleteRegionModelList, manager,
                                player_sex, placements, placement_count);
  GetInstance().AddMissingModels(manager, placements, placement_count);
}

void MapCharacter::LogShippedCharacters(const MapEventData* events) const {
  const u32 count = events->character_count;
  const CharacterPlacement* shipped = events->characters;
  ui::LogApplication::Print(u"map=%u chars=%u", events->map_id, count);
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

void MapCharacter::PlaceCharacters(MapEventData* events) {
  const MapId map_id = events->map_id;
  const CharacterPlacement* shipped = events->characters;
  u32 shipped_count = events->character_count;
  if (shipped == nullptr) return;
  if (shipped_count > kMaxCharactersPerMap) return;

  s32 graft_dx = 0;
  s32 graft_dz = 0;
  const bool is_grafted = MapGraft::GetTileOffset(map_id, graft_dx, graft_dz);

  if (IsEmptied(map_id)) shipped_count = 0;

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

  if (added == 0 && !is_grafted && !IsEmptied(map_id)) return;

  events->characters = placements_;
  events->character_count = count;
  events->character_capacity = count;

  if (is_logging_enabled) {
    ui::LogApplication::Print(u"placed %u -> chars=%u", added, count);
  }
}

bool MapCharacter::IsEmptied(MapId map_id) const {
  for (u32 i = 0; i < emptied_count_; i++) {
    if (emptied_maps_[i] == map_id) return true;
  }
  return false;
}

void MapCharacter::BuildPlacement(const MapCharacterRequest& request, u16 local_id,
                           CharacterPlacement* out) {
  *out = CharacterPlacement{};
  out->local_id = local_id;
  out->model_id = request.model_id;
  out->movement_id = request.movement_id;
  out->interaction_kind = 0;
  out->hide_when_flag_set = request.hide_when_flag_set;
  out->script_id = request.script_id;
  out->facing = request.facing;
  out->displayed_on_map_id = request.map_id;
  out->owned_by_map_id = request.map_id;
  out->map_id = request.map_id;
  out->position.kind = PositionKind::kTileGrid;
  out->position.tile_x = request.tile_x;
  out->position.tile_z = request.tile_z;
  out->position.height = request.height;
}

void MapCharacter::AddMissingModels(CharacterManager* manager,
                                    const CharacterPlacement* placements,
                                    u32 placement_count) {
  if (placements == nullptr) return;

  ModelAppearance* models = manager->models;
  u32 count = manager->model_count;
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
  manager->model_count = count;
  manager->model_count_original = count;
}

bool MapCharacter::Contains(const ModelAppearance* models, u32 count,
                     ModelId model_id) {
  for (u32 i = 0; i < count; i++) {
    if (models[i].model_id == model_id) return true;
  }
  return false;
}

bool MapCharacter::LoadAppearance(const CharacterManager* manager,
                                  ModelId model_id, ModelAppearance* out) {
  s32 index = ((s32 (*)(ModelId))address::kGetModelArchiveIndex)(
      model_id);
  if (index < 0) return false;

  void* archive = manager->model_param_archive;
  if (archive == nullptr) return false;

  ((void (*)(void*, s32, void*))sys::address::kArchiveLoadData2)(archive, index,
                                                             out);
  return true;
}

} // namespace overworld
