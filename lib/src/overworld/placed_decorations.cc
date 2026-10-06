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

#include "overworld/patch/placed_decorations.h"

#include <cmath>

#include "battle/constant/background.h"
#include "battle/constant/ground.h"
#include "core/native/process_manager.h"
#include "core/utils.h"
#include "overworld/constant/position_kind.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/tile_editor.h"
#include "renderer/native/h3d_shader_model.h"
#include "script/patch/native_script.h"
#include "ui/log_application.h"

namespace overworld {
namespace {
constexpr f32 kPi = 3.14159265f;
constexpr u32 kMinBlockingLayout = 2;
constexpr u32 kTalkDirectionAll = 4;

constexpr u32 kTileImpassableBit = 1;
constexpr u32 kTileEncountersBit = 1u << 2;
constexpr u32 kGroundIdMask = 0xFFu << 24;
constexpr u32 kBlockedGroundBits = 1u << 24;
constexpr u32 kTallGrassGroundBits = 0x20u << 24;
constexpr u32 kTileBackgroundShift = 17;
constexpr u32 kTileBackgroundMask = 0x7Fu << kTileBackgroundShift;

constexpr StaticEncounterId kBorrowedEncounter = StaticEncounterId::kWurmple;
constexpr u32 kTallGrassPokemonLevel = 100;
constexpr battle::BackgroundId kTallGrassBackground =
    battle::BackgroundId::kShore;
constexpr battle::GroundId kTallGrassGround = battle::GroundId::kShore;
const MoveId kTallGrassMoves[4] = {MoveId::kRoarOfTime, MoveId::kSpacialRend,
                                   MoveId::kShadowForce, MoveId::kJudgment};

#include "overworld/data/decoration_names.inc"

u32 Utf8ToUtf16(const char* text, c16* out, u32 capacity) {
  u32 length = 0;
  for (const u8* p = reinterpret_cast<const u8*>(text);
       *p != 0 && length + 1 < capacity; p++) {
    u32 code = *p;
    if ((code & 0xE0) == 0xC0 && p[1] != 0) {
      code = ((code & 0x1F) << 6) | (p[1] & 0x3F);
      p++;
    }
    out[length++] = static_cast<c16>(code);
  }
  out[length] = u'\0';
  return length;
}

template <u32 Slot>
void TalkScript(script::Context& script) {
  PlacedDecorations::RunTalkScript(script, Slot);
}

const script::Function kTalkScripts[PlacedDecorations::kMaxDecorations] = {
    TalkScript<0>, TalkScript<1>, TalkScript<2>, TalkScript<3>,
    TalkScript<4>, TalkScript<5>, TalkScript<6>, TalkScript<7>,
};

bool IsSideways(Facing facing) {
  return facing == Facing::kLeft || facing == Facing::kRight;
}

Facing ToCardinalFacing(Facing facing) {
  switch (facing) {
    case Facing::kUp:
    case Facing::kLeft:
    case Facing::kRight:
      return facing;
    default:
      return Facing::kDown;
  }
}

f32 ModelRotationOf(Facing facing) {
  switch (facing) {
    case Facing::kUp:
      return kPi;
    case Facing::kLeft:
      return -kPi / 2.0f;
    case Facing::kRight:
      return kPi / 2.0f;
    default:
      return 0.0f;
  }
}
} // namespace

void PlacedDecorations::Initialize() {
  for (u32 i = 0; i < kMaxDecorations; i++) {
    script::NativeScript::Register(
        static_cast<ScriptId>(kFirstTalkScript + i), kTalkScripts[i]);
  }
}

bool PlacedDecorations::ReadDecorationInfo(u16 index, u8* width, u8* depth,
                                           bool* blocks_movement) {
  if (!address::kDecorationTable || index >= address::kDecorationCount) {
    return false;
  }
  const u8* info = reinterpret_cast<const u8*>(address::kDecorationTable) +
                   index * address::kDecorationEntrySize;
  const u16 size_x = *reinterpret_cast<const u16*>(info + 0);
  const f32 model_height = *reinterpret_cast<const f32*>(info + 4);
  const u16 size_z = *reinterpret_cast<const u16*>(info + 8);
  const u8 layout = info[14];

  *width = size_x == 0 ? 1 : (size_x > 5 ? 5 : size_x);
  *depth = size_z == 0 ? 1 : (size_z > 5 ? 5 : size_z);
  *blocks_movement = layout >= kMinBlockingLayout && model_height > 0.5f;
  return true;
}

bool PlacedDecorations::Add(const DecorationRequest& request) {
  auto& ctx = GetInstance();
  if (ctx.count_ >= kMaxDecorations) return false;

  Entry& entry = ctx.entries_[ctx.count_];
  entry = Entry{};
  entry.request = request;
  entry.request.facing = ToCardinalFacing(request.facing);

  u8 width = 1;
  u8 depth = 1;
  if (!ReadDecorationInfo(static_cast<u16>(request.decoration), &width, &depth,
                          &entry.blocks_movement)) {
    return false;
  }
  entry.is_tall_grass = request.decoration == DecorationId::kTallGrass;
  if (entry.is_tall_grass) {
    entry.blocks_movement = false;
    entry.request.is_talkable = false;
  }
  const bool is_sideways = IsSideways(entry.request.facing);
  entry.width = is_sideways ? depth : width;
  entry.depth = is_sideways ? width : depth;
  ctx.count_++;
  return true;
}

bool PlacedDecorations::AddInFrontOfPlayer(DecorationId decoration,
                                           bool is_talkable) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);

  s32 step_x = 0;
  s32 step_z = 0;
  const Vec3& look = player.facing_direction;
  if (std::fabs(look.x) > std::fabs(look.z)) {
    step_x = look.x > 0 ? 1 : -1;
  } else {
    step_z = look.z > 0 ? 1 : -1;
  }

  Facing facing = Facing::kDown;
  if (step_z > 0) facing = Facing::kUp;
  if (step_x > 0) facing = Facing::kLeft;
  if (step_x < 0) facing = Facing::kRight;

  u8 width = 1;
  u8 depth = 1;
  bool blocks_movement = false;
  if (!ReadDecorationInfo(static_cast<u16>(decoration), &width, &depth,
                          &blocks_movement)) {
    return false;
  }
  if (IsSideways(facing)) {
    const u8 previous_width = width;
    width = depth;
    depth = previous_width;
  }

  s32 tile_x = player_x + step_x;
  s32 tile_z = player_z + step_z;
  if (step_z != 0) tile_x -= (width - 1) / 2;
  if (step_x != 0) tile_z -= (depth - 1) / 2;
  if (step_z < 0) tile_z -= depth - 1;
  if (step_x < 0) tile_x -= width - 1;

  DecorationRequest request;
  request.map_id = MapManager::GetInstance().GetMap();
  request.decoration = decoration;
  request.tile_x = static_cast<s16>(tile_x);
  request.tile_z = static_cast<s16>(tile_z);
  request.ground_height = player.GetDrawModel().position.y;
  request.facing = facing;
  request.is_talkable = is_talkable;
  if (!Add(request)) {
    ui::LogApplication::Print(u"decorations: list full (max %u)",
                              kMaxDecorations);
    return false;
  }

  if (is_talkable) ReloadMap();
  return true;
}

void PlacedDecorations::Clear() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.count_; i++) {
    Entry& entry = ctx.entries_[i];
    RestoreTiles(entry);
    pokemon::ModelLoader::Drop(&entry.model);
    entry = Entry{};
  }
  ctx.count_ = 0;
}

u32 PlacedDecorations::GetCount() { return GetInstance().count_; }

void PlacedDecorations::Update() {
  auto& ctx = GetInstance();
  ctx.frame_++;

  if (ctx.is_battle_starting_) {
    if (core::ProcessManager::IsBattleActive()) {
      ctx.is_battle_seen_ = true;
    } else if (core::ProcessManager::IsOverworldActive() &&
               (ctx.is_battle_seen_ ||
                ctx.frame_ - ctx.battle_start_frame_ > kBattleTimeoutFrames)) {
      ctx.is_battle_starting_ = false;
      ctx.is_battle_seen_ = false;
      if (ctx.is_tall_grass_battle_pending_) RestoreBorrowedEncounter();
    }
  }
  if (ctx.count_ == 0) return;

  if (!core::ProcessManager::IsOverworldActive()) {
    for (u32 i = 0; i < ctx.count_; i++) DiscardModel(ctx.entries_[i]);
    return;
  }

  if (MapManager::GetInstance().GetNextMapId() != MapManager::kNoMap) {
    for (u32 i = 0; i < ctx.count_; i++) DiscardModel(ctx.entries_[i]);
    return;
  }
  if (ctx.is_battle_starting_) return;
  if (ctx.frame_ - ctx.map_load_frame_ < kShowDelayFrames) return;

  const MapId map = MapManager::GetInstance().GetMap();

  if (IsNearExit(map)) {
    for (u32 i = 0; i < ctx.count_; i++) HideModel(ctx.entries_[i]);
    return;
  }
  for (u32 i = 0; i < ctx.count_; i++) {
    Entry& entry = ctx.entries_[i];
    if (entry.request.map_id != map) continue;
    if (!entry.is_shown) ShowModel(entry);
    if (entry.is_tall_grass ||
        (ctx.is_collision_enabled && entry.blocks_movement)) {
      ApplyTileChanges(entry);
    } else if (entry.blocks_movement) {
      RestoreTiles(entry);
    }
  }
  UpdateTallGrass();
}

void PlacedDecorations::ShowModel(Entry& entry) {
  if (entry.show_attempts >= kMaxShowAttempts) return;
  entry.show_attempts++;

  const f32 tile_size = static_cast<f32>(WorldLayout::kUnitsPerTile);
  const Vec3 position((entry.request.tile_x + entry.width / 2.0f) * tile_size,
                      entry.request.ground_height,
                      (entry.request.tile_z + entry.depth / 2.0f) * tile_size);
  if (!pokemon::ModelLoader::LoadDecoration(
      &entry.model, static_cast<u32>(entry.request.decoration), position)) {
    return;
  }

  const Vec3 rotation(0.0f, ModelRotationOf(entry.request.facing), 0.0f);
  entry.model.model->SetRotate(rotation);
  entry.is_shown = true;
}

void PlacedDecorations::HideModel(Entry& entry) {
  if (!entry.is_shown) return;
  RestoreTiles(entry);
  pokemon::ModelLoader::Drop(&entry.model);
  entry.is_shown = false;
  entry.show_attempts = 0;
}

void PlacedDecorations::DiscardModel(Entry& entry) {
  if (!entry.is_shown) return;
  pokemon::ModelLoader::Untrack(&entry.model);
  entry.model = pokemon::LoadedModel{};
  entry.is_shown = false;
  entry.show_attempts = 0;
  entry.saved_tile_mask = 0;
}

bool PlacedDecorations::IsNearExit(MapId map) {
  auto& ctx = GetInstance();
  if (ctx.exit_map_ != map) return false;
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);
  for (u32 i = 0; i < ctx.exit_count_; i++) {
    const s32 distance_x = player_x - ctx.exit_tile_x_[i];
    const s32 distance_z = player_z - ctx.exit_tile_z_[i];
    if (distance_x >= -kExitSafeDistance && distance_x <= kExitSafeDistance &&
        distance_z >= -kExitSafeDistance && distance_z <= kExitSafeDistance) {
      return true;
    }
  }
  return false;
}

bool PlacedDecorations::IsOnTallGrass(s32 tile_x, s32 tile_z) {
  auto& ctx = GetInstance();
  const MapId map = MapManager::GetInstance().GetMap();
  for (u32 i = 0; i < ctx.count_; i++) {
    const Entry& entry = ctx.entries_[i];
    if (!entry.is_tall_grass || entry.request.map_id != map) continue;
    if (tile_x >= entry.request.tile_x &&
        tile_x < entry.request.tile_x + entry.width &&
        tile_z >= entry.request.tile_z &&
        tile_z < entry.request.tile_z + entry.depth) {
      return true;
    }
  }
  return false;
}

void PlacedDecorations::UpdateTallGrass() {
  auto& ctx = GetInstance();
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);

  if (!IsOnTallGrass(player_x, player_z)) {
    ctx.tall_grass_tile_x_ = -1;
    ctx.tall_grass_tile_z_ = -1;
    ctx.step_countdown_ = 0;
    return;
  }
  if (ctx.is_tall_grass_battle_pending_) return;

  if (player_x != ctx.tall_grass_tile_x_ || player_z != ctx.
      tall_grass_tile_z_) {
    ctx.tall_grass_tile_x_ = player_x;
    ctx.tall_grass_tile_z_ = player_z;
    ctx.step_countdown_ = 0;
    if (core::Utils::GetRandomValue(100) <
        ctx.tall_grass_battle_chance_percent) {
      ctx.step_countdown_ = kTallGrassStepDelayFrames;
    }
  } else if (ctx.step_countdown_ > 0 && --ctx.step_countdown_ == 0) {
    StartTallGrassBattle();
  }
}

void PlacedDecorations::RemoveModelsBeforeBattle() {
  auto& ctx = GetInstance();
  if (ctx.count_ == 0) return;
  for (u32 i = 0; i < ctx.count_; i++) HideModel(ctx.entries_[i]);
  ctx.is_battle_starting_ = true;
  ctx.is_battle_seen_ = false;
  ctx.battle_start_frame_ = ctx.frame_;
}

void PlacedDecorations::StartTallGrassBattle() {
  auto& ctx = GetInstance();
  if (!address::kCallStaticEncounter) return;
  RemoveModelsBeforeBattle();

  StaticEncounter& encounter = StaticEncounter::GetInstance(kBorrowedEncounter);
  if (!ctx.has_saved_encounter_) {
    ctx.saved_encounter_ = encounter;
    ctx.has_saved_encounter_ = true;
  }
  encounter.species = static_cast<SpeciesId>(929);
  encounter.form = FormId::kMega;
  encounter.level = kTallGrassPokemonLevel;
  encounter.is_shiny = ShinyRoll::kNotShiny;
  encounter.kind = StaticEncounterKind::kNormal;
  encounter.background = kTallGrassBackground;
  encounter.ground = kTallGrassGround;
  encounter.animation = EncounterAnimationId::kKyogre;

  auto& randomizer = StaticRandomizer::GetInstance();
  const bool was_randomizing = randomizer.randomize_species;
  randomizer.randomize_species = false;
  ((s32(*)(core::GameManager*, StaticEncounterId, u32, s32))
    address::kCallStaticEncounter)(&core::GameManager::GetInstance(),
                                   kBorrowedEncounter, 0, -1);
  randomizer.randomize_species = was_randomizing;

  ctx.is_tall_grass_battle_pending_ = true;
}

void PlacedDecorations::RestoreBorrowedEncounter() {
  auto& ctx = GetInstance();
  if (ctx.has_saved_encounter_) {
    StaticEncounter::GetInstance(kBorrowedEncounter) = ctx.saved_encounter_;
    ctx.has_saved_encounter_ = false;
  }
  ctx.is_tall_grass_battle_pending_ = false;
}

void PlacedDecorations::OnWildPokemonRolled(WildPokemon* pokemons, u32 count) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  if (!IsOnTallGrass(static_cast<s32>(player.map_pos.coords.x),
                     static_cast<s32>(player.map_pos.coords.z))) {
    return;
  }
  for (u32 i = 0; i < count; i++) {
    pokemons[i].species = SpeciesId::kChansey;
    pokemons[i].form = FormId::kNormal;
    pokemons[i].level = kTallGrassPokemonLevel;
    pokemons[i].is_shiny = false;
    for (u32 move = 0; move < 4; move++) {
      pokemons[i].moves[move] = kTallGrassMoves[move];
    }
  }
}

void PlacedDecorations::ApplyTileChanges(Entry& entry) {
  TileEditor::Block blocks[TileEditor::kMaxBlocks];
  const u32 count = TileEditor::CollectBlocks(blocks, TileEditor::kMaxBlocks);
  if (count == 0) return;

  for (u32 z = 0; z < entry.depth; z++) {
    for (u32 x = 0; x < entry.width; x++) {
      const u32 index = z * entry.width + x;
      if (index >= kMaxFootprintTiles) continue;
      u32* attr_slot = TileEditor::Slot(blocks, count, entry.request.tile_x + x,
                                        entry.request.tile_z + z);
      if (attr_slot == nullptr) continue;
      if ((entry.saved_tile_mask & (1u << index)) == 0) {
        entry.saved_tile_mask |= 1u << index;
        entry.original_tile_attrs[index] = *attr_slot;
      }
      u32 attr = entry.original_tile_attrs[index];
      if (entry.is_tall_grass) {
        attr = (attr & ~kGroundIdMask & ~kTileImpassableBit &
                ~kTileBackgroundMask) |
               kTallGrassGroundBits | kTileEncountersBit |
               (static_cast<u32>(kTallGrassBackground)
                << kTileBackgroundShift);
      } else {
        attr = (attr & ~kGroundIdMask) | kBlockedGroundBits |
               kTileImpassableBit;
      }
      *attr_slot = attr;
    }
  }
}

void PlacedDecorations::RestoreTiles(Entry& entry) {
  if (entry.saved_tile_mask == 0) return;
  TileEditor::Block blocks[TileEditor::kMaxBlocks];
  const u32 count = TileEditor::CollectBlocks(blocks, TileEditor::kMaxBlocks);
  if (count == 0) return;

  for (u32 z = 0; z < entry.depth; z++) {
    for (u32 x = 0; x < entry.width; x++) {
      const u32 index = z * entry.width + x;
      if (index >= kMaxFootprintTiles) continue;
      if ((entry.saved_tile_mask & (1u << index)) == 0) continue;
      u32* attr_slot = TileEditor::Slot(blocks, count, entry.request.tile_x + x,
                                        entry.request.tile_z + z);
      if (attr_slot != nullptr) *attr_slot = entry.original_tile_attrs[index];
    }
  }
  entry.saved_tile_mask = 0;
}

void PlacedDecorations::OnMapEventsLoaded(MapEventData* events) {
  auto& ctx = GetInstance();
  ctx.map_load_frame_ = ctx.frame_;
  if (events == nullptr) return;

  ctx.exit_map_ = events->map_id;
  ctx.exit_count_ = 0;
  const u32 grid_position = static_cast<u32>(PositionKind::kTileGrid);
  const s32 tile_size = static_cast<s32>(WorldLayout::kUnitsPerTile);
  for (u32 i = 0; events->warps != nullptr && i < events->warp_count &&
                  ctx.exit_count_ < kMaxExits;
       i++) {
    const WarpEvent& exit = events->warps[i];
    if (exit.position_kind != grid_position) continue;
    ctx.exit_tile_x_[ctx.exit_count_] = exit.world.x / tile_size;
    ctx.exit_tile_z_[ctx.exit_count_] = exit.world.z / tile_size;
    ctx.exit_count_++;
  }
  if (ctx.count_ == 0) return;

  for (u32 i = 0; i < ctx.count_; i++) DiscardModel(ctx.entries_[i]);

  u32 total = events->sign_count;
  if (total > kMaxTalkEvents) return;
  for (u32 i = 0; i < ctx.count_; i++) {
    const Entry& entry = ctx.entries_[i];
    if (entry.request.is_talkable && entry.request.map_id == events->map_id) {
      total++;
    }
  }
  if (total == events->sign_count || total > kMaxTalkEvents) return;

  u32 next = 0;
  for (; next < events->sign_count && events->signs != nullptr; next++) {
    ctx.talk_events_[next] = events->signs[next];
  }
  for (u32 i = 0; i < ctx.count_; i++) {
    const Entry& entry = ctx.entries_[i];
    if (!entry.request.is_talkable || entry.request.map_id != events->map_id) {
      continue;
    }
    SignEvent& talk_event = ctx.talk_events_[next++];
    talk_event = SignEvent{};
    talk_event.id = static_cast<u16>(kFirstTalkScript + i);
    talk_event.kind = 0;
    talk_event.talk_facing = static_cast<Facing>(kTalkDirectionAll);
    talk_event.position_kind = static_cast<u16>(PositionKind::kTileGrid);
    talk_event.tiles.tile_x = entry.request.tile_x;
    talk_event.tiles.tile_z = entry.request.tile_z;
    talk_event.tiles.width = entry.width;
    talk_event.tiles.depth = entry.depth;
    talk_event.tiles.height = static_cast<s32>(entry.request.ground_height);
  }
  events->signs = ctx.talk_events_;
  events->sign_count = static_cast<u16>(next);
}

void PlacedDecorations::ReloadMap() {
  const Position& position = ModelManager::GetInstance().GetPlayer().world_pos;
  MapManager::ChangeMap(MapManager::GetInstance().GetMap(), position,
                        Facing::kUp, true, false);
}

void PlacedDecorations::RunTalkScript(script::Context& script, u32 slot) {
  auto& ctx = GetInstance();
  if (slot >= ctx.count_) return;

  const u32 index = static_cast<u32>(ctx.entries_[slot].request.decoration);
  c16 text[80] = u"It's a ";
  u32 length = 0;
  while (text[length] != u'\0') length++;
  if (index < SIZE(DECORATION_NAMES)) {
    length += Utf8ToUtf16(DECORATION_NAMES[index], text + length,
                          SIZE(text) - length - 2);
  }
  text[length++] = u'.';
  text[length] = u'\0';

  script.Talk(text, script::WindowType::kSign);
}
} // namespace overworld