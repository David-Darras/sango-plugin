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
#include "pokemon/patch/model_loader.h"

namespace overworld {

/// Covers the walkable tiles around the player with the game's own
/// encounter grass models. The models and their textures are loaded once
/// and every patch is a cheap instance of them.
class FieldGrass {
  MAKE_SINGLETON(FieldGrass)

public:
  enum Kind : u8 {
    kGreen, ///< Plain grass
    kFern, ///< Tall grass / ferns
    kAsh, ///< Ash-covered grass
    kKindCount,
  };

  enum Mix : u32 {
    kMixAll,
    kMixGreen,
    kMixFern,
    kMixAsh,
    kMixCount,
  };

  static constexpr u32 kMaxPatches = 96;
  static constexpr u32 kMaxRadius = 100;

  bool is_enabled = false;
  u32 mix = kMixAll;
  u32 radius = 5; ///< Tiles around the player that get grass
  u32 density = 60; ///< Percent of the walkable tiles that get a patch

  static void Update();
  static void RemoveModelsBeforeBattle();
  static u32 GetPatchCount();

private:
  struct Patch {
    pokemon::LoadedModel model;
    s32 tile_x = 0;
    s32 tile_z = 0;
    Kind kind = kGreen;
    bool is_used = false;
  };

  static constexpr u32 kShowDelayFrames = 20;
  static constexpr u32 kRefreshFrames = 6;
  static constexpr u32 kBuildsPerRefresh = 3;
  static constexpr u32 kBattleTimeoutFrames = 1800;
  static constexpr f32 kHiddenHeight = -10000.0f;

  static bool LoadResources();
  static void Refresh();
  static Kind PickKind(u32 hash);
  static Patch* FindPatch(s32 tile_x, s32 tile_z);
  static Patch* TakeFreePatch(Kind kind, u32* builds_left);
  static Patch* StealFarPatch(Kind kind, s32 player_x, s32 player_z,
                              s32 max_distance);
  static void Release(Patch& patch);
  static void DropAll();
  static void DiscardAll();

  Patch patches_[kMaxPatches];
  void* pack_ = nullptr;
  renderer::H3dResource* resources_[kKindCount] = {};
  u32 frame_ = 0;
  u32 map_load_frame_ = 0;
  u32 last_map_ = 0xFFFFFFFF;
  bool is_battle_starting_ = false;
  bool is_battle_seen_ = false;
  u32 battle_start_frame_ = 0;
};
} // namespace overworld
