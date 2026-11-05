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
 * @file tall_grass.h
 * @brief Shows 3D tall grass around the player.
 */

#pragma once

#include "common.h"
#include "pokemon/patch/model_loader.h"

namespace overworld {

/// Puts tall grass models on the tiles around the player. The library loads
/// the models one time. Each patch of grass is a copy of a model.
///
/// The player can also place grass by hand: the 5 kinds of grass of Pokémon X
/// and Y (the pack sango/grass/grass.pc), on the tiles in front of the player.
/// The list is saved for each world in sango/maps/layout_<id>.grass.
class TallGrass {
  MAKE_SINGLETON(TallGrass)

public:
  enum Kind : u8 {
    kGreen, ///< Short grass.
    kFern, ///< Tall grass and ferns.
    kAsh, ///< Grass under ash.
    kKindCount,
  };

  enum Mix : u32 {
    kMixAll,
    kMixGreen,
    kMixFern,
    kMixAsh,
    kMixCount,
  };

  static constexpr u32 kXyKindCount = 5;
  static constexpr u32 kMaxPlaced = 256;
  static constexpr u32 kMaxShown = 48;
  static constexpr u32 kPlacedRadius = 12; ///< The distance in tiles.
  static constexpr u32 kMaxSide = 8; ///< The largest width or depth to place.
  static constexpr u32 kMaxPatches = 96;
  static constexpr u32 kMaxRadius = 100;

  bool is_enabled = false;
  u32 mix = kMixAll;
  u32 radius = 5; ///< The distance in tiles around the player.
  u32 density = 60; ///< The percent of the walkable tiles that get grass.

  /// The kind of grass of X and Y that the menu places (0 to 4).
  u32 placed_kind = 0;
  /// The light set of the placed grass (0 to 31). The loader gives the
  /// Pokémon light (15) to its models; the grass of the field uses another one.
  u32 placed_light_set = 1;
  u32 placed_width = 1;
  u32 placed_depth = 1;

  /// Places the grass of X and Y on the tiles in front of the player (a
  /// rectangle of placed_width x placed_depth tiles). A tile that has grass
  /// gets the new kind. Returns false when the list is full.
  static bool PlaceInFrontOfPlayer();
  /// Places a square of side x side tiles of the kind that the menu has, in
  /// front of the player. The width and the depth of the menu do not change.
  static bool PlaceSquareInFrontOfPlayer(u32 side);
  /// Removes the placed grass from the same rectangle.
  static void RemoveInFrontOfPlayer();
  /// Removes all the grass that the player placed in this world.
  static void RemoveAllPlaced();
  static u32 GetPlacedCount();

  static void Update();
  /// Removes the grass before a battle starts.
  static void RemoveModelsBeforeBattle();

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

  /// A grass of X and Y that the player placed. The file has these records.
  struct Placed {
    s16 tile_x = 0;
    s16 tile_z = 0;
    u8 kind = 0;
    u8 reserved = 0;
  };

  struct XyPatch {
    pokemon::LoadedModel model;
    s32 tile_x = 0;
    s32 tile_z = 0;
    u8 kind = 0;
    bool is_used = false;
    bool was_player_on = false;
    u8 sway_frames = 0; ///< The frames that are left of the sway.
    s32 light_set = -1; ///< The light set that the model has now.
    f32 yaw = 0.0f;
  };

  static constexpr u32 kPackCapacity = 0x30000;
  static constexpr u32 kPackAttempts = 3;
  static constexpr u8 kSwayFrames = 60;

  static bool LoadResources();
  static bool LoadXyResources();
  static void RefreshPlaced();
  /// Makes the grass sway when the player walks into it.
  static void UpdateSway();
  static void LoadPlaced(u32 layout);
  static void SavePlaced();
  static s32 FindPlaced(s32 tile_x, s32 tile_z);
  static bool Upsert(s32 tile_x, s32 tile_z, u8 kind);
  static u32 GetFrontTiles(s32* tiles_x, s32* tiles_z);
  static XyPatch* FindXyPatch(s32 tile_x, s32 tile_z);
  static XyPatch* TakeFreeXyPatch(u8 kind, u32* builds_left);
  static void ReleaseXy(XyPatch& patch);
  static void DropXy();
  static void DiscardXy();
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
  XyPatch xy_patches_[kMaxShown];
  Placed placed_[kMaxPlaced];
  u32 placed_count_ = 0;
  u32 placed_layout_ = 0xFFFFFFFF;
  void* xy_pack_ = nullptr;
  renderer::H3dResource* xy_resources_[kXyKindCount] = {};
  bool is_xy_missing_ = false;
  u32 xy_failures_ = 0;
  renderer::H3dResource* resources_[kKindCount] = {};
  u32 frame_ = 0;
  u32 map_load_frame_ = 0;
  u32 last_map_ = 0xFFFFFFFF;
  bool is_battle_starting_ = false;
  bool is_battle_seen_ = false;
  u32 battle_start_frame_ = 0;
};
} // namespace overworld
