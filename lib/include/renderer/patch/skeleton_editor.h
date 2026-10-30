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
 * @file skeleton_editor.h
 * @brief Moves, turns and scales the bones of the 3D models.
 */

#pragma once

#include "common.h"

namespace renderer {
struct H3dModel;

/// The change of one bone: a move, a turn and a scale, in the space of the bone.
struct BoneChange {
  f32 translate[3] = {0, 0, 0};
  f32 rotate[3] = {0, 0, 0}; ///< In degrees.
  f32 scale[3] = {1, 1, 1};

  bool IsIdentity() const {
    for (u32 i = 0; i < 3; i++) {
      if (translate[i] != 0 || rotate[i] != 0 || scale[i] != 1) return false;
    }
    return true;
  }
};

/**
 * @brief Changes the bones of the 3D models.
 *
 * A model has a skeleton: bones that the animations of the game move. The
 * game gives each bone a matrix in the world, and it draws the model with these
 * matrices. The editor changes the matrices when the game starts to draw the
 * model, so it works for all the models, whoever calculates the bones (the
 * animations, the motion system of the characters...). A change of a bone
 * moves, turns or scales the bone in its own space, and the bones under it
 * follow, as in a 3D program.
 *
 * A change is for all the models that have the same skeleton: the key is a
 * hash of the names of the bones. The file sdmc:/sango/skeleton.ini keeps the
 * changes, and the editor reads it at the start. ORAS only.
 */
class SkeletonEditor {
  MAKE_SINGLETON(SkeletonEditor)
public:
  static constexpr u32 kMaxEdits = 32;
  static constexpr u32 kMaxNameLength = 24;

  /// Installs the hook and reads the saved changes.
  static void Initialize();

  /// Tells the editor that the game destroys a model.
  static void OnModelRemoved(const H3dModel* model);

  static u32 GetBoneCount(const H3dModel* model);
  /// The name of a bone, or an empty text.
  static const c8* GetBoneName(const H3dModel* model, u32 bone);

  /// Gives the matrices of the bones of a model in the world (12 numbers for
  /// each bone: 3 rows of 4) and the parent of each bone. Returns null when the
  /// model has no readable skeleton. @a count gets the number of bones.
  static f32* GetWorldMatrices(const H3dModel* model, u32* count,
                               const u16** parents);

  /// Returns the change of a bone, or null.
  static const BoneChange* GetChange(const H3dModel* model, u32 bone);

  /// Keeps the change of a bone. Returns false when the list is full.
  static bool SetChange(const H3dModel* model, u32 bone,
                        const BoneChange& change);

  /// Removes the change of a bone.
  static void RemoveChange(const H3dModel* model, u32 bone);

  /// Removes all the changes of the skeleton of a model.
  static void RemoveAll(const H3dModel* model);

  /// The number of changes of all the skeletons.
  static u32 GetChangeCount();

  /**
   * @brief Puts the legs of a character in a sit pose, until ClearSeatPose().
   *
   * The model is the one that the two objects of the character (@a draw_model
   * and @a character) point to, and it is not @a excluded. The editor finds
   * the legs by the shape of the skeleton: the two lowest chains of bones that
   * join in the hips. The thighs go forward (@a forward_x and @a forward_z,
   * a direction on the ground) and to the sides by @a leg_spread (0 is none,
   * 2 is a lot), and the lower legs go down. The arms go forward, bent, as if
   * they hold a mane.
   *
   * @a is_moving tells that the character walks. The hips then stay at the
   * height that they have at rest, over @a ground_y (the height of the feet
   * that the game gives): the bounce of the walk animation does not make the
   * character hop. The whole character goes up by @a raise: the game moves the
   * player on the ground, and the pose lifts the drawn model only.
   */
  static void SetSeatPose(const void* draw_model, u32 draw_model_size,
                          const void* character, u32 character_size,
                          const H3dModel* excluded, f32 forward_x,
                          f32 forward_z, bool is_moving, f32 raise,
                          f32 leg_spread, f32 ground_y);

  /// The legs go back to the animation of the game.
  static void ClearSeatPose();

  /// Writes the changes to sdmc:/sango/skeleton.ini. Returns true on success.
  static bool Save();

private:
  static constexpr u32 kMaxModels = 48;
  static constexpr u32 kMaxRuntimeBones = 128;
  static constexpr u32 kMaxRuntimes = 8;

  struct Edit {
    bool is_used;
    u32 skeleton; ///< The hash of the names of the bones.
    u32 bone;
    c8 name[kMaxNameLength];
    BoneChange change;
  };

  /// A model that the editor saw: its skeleton, to find the changes fast.
  struct ModelInfo {
    const H3dModel* model;
    u32 skeleton;
    u32 bone_count;
  };

  /// The matrices that the editor changed for a model: the matrices as the
  /// game wrote them, and the matrices after the changes. The editor compares
  /// them with the matrices of the model to know if the game wrote new ones.
  struct Runtime {
    const H3dModel* model;
    u32 skeleton;
    u32 bone_count;
    u32 version;
    bool is_valid;
    f32 original[kMaxRuntimeBones * 12];
    f32 edited[kMaxRuntimeBones * 12];
  };

  static void MeshDrawHook(void* mesh, void* helper, void* state, void* extra);
  static bool Load();
  /// Gives the changes to the matrices of a model, one time for each
  /// calculation of the game.
  void ApplyToModel(const H3dModel* model);
  /// true: the model is the character with the sit pose.
  bool IsSeatModel(const H3dModel* model, u32 count);
  /// Bends the legs of the model. Returns false when no legs are found.
  bool ApplySeatPose(const H3dModel* model, f32* worlds, const u16* parents,
                     u32 count);
  u32 GetSkeleton(const H3dModel* model);
  Edit* Find(u32 skeleton, u32 bone);

  Edit edits_[kMaxEdits] = {};
  ModelInfo models_[kMaxModels] = {};
  Runtime runtimes_[kMaxRuntimes] = {};
  u32 next_model_ = 0;
  u32 next_runtime_ = 0;
  u32 edit_count_ = 0;
  u32 version_ = 1; ///< It changes when a change changes: the matrices follow.
  bool has_runtime_ = false;
  bool has_seat_pose_ = false;
  const void* seat_owner_[2] = {nullptr, nullptr};
  u32 seat_owner_size_[2] = {0, 0};
  const H3dModel* seat_model_ = nullptr; ///< The character, when it is found.
  const H3dModel* seat_excluded_ = nullptr;
  f32 seat_forward_[2] = {0.0f, 1.0f};
  bool seat_moving_ = false;
  f32 seat_raise_ = 0.0f;
  f32 seat_spread_ = 0.25f;
  f32 seat_ground_y_ = 0.0f;
  f32 seat_rest_height_ = -1.0f; ///< The height of the hips at rest (< 0: unknown).
  f32 seat_walk_height_ = -1.0f; ///< The same, filtered, while the character walks.
  const H3dModel* seat_checked_[8] = {}; ///< The models that are not the character.
  u32 seat_checked_count_ = 0;
};

} // namespace renderer
