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
 * @file model_viewer.h
 * @brief Lists the 3D models of the screen, and changes their parts and their
 *        materials.
 */

#pragma once

#include "common.h"
#include "core/color.h"

namespace renderer {
struct H3dModel;

/// The values of a material that the menu changes.
struct MaterialState {
  Color ambient;
  Color diffuse;
  u8 culling = 0; ///< 0: no culling. 1 and 2: the two sides that the GPU hides.
  bool has_alpha_test = false;
};

/**
 * @brief The 3D models of the current screen.
 *
 * A 3D model has parts (the meshes that the game shows or hides, for example a
 * hat) and materials (the colors, the textures and the blending of the
 * surfaces). The viewer keeps a list of the models that the game adds to its
 * scenes, so that the menu can show them. A change stays in the model until
 * the game destroys it. ORAS only.
 */
class ModelViewer {
  MAKE_SINGLETON(ModelViewer)
public:
  static constexpr u32 kMaxModels = 40;

  /// Installs the hook that removes the models that the game destroys.
  static void Initialize();

  /// Adds a model that the game puts in a scene.
  static void OnModelAdded(H3dModel* model);

  /// Writes the models of the screen. Returns their number.
  static u32 GetModels(H3dModel** out, u32 max);

  /// Returns true when the model is in the list. The menu uses it before it
  /// reads a model that it kept: the game can destroy the model.
  static bool IsKnown(const H3dModel* model);

  /// The kind of a model, for a title: "Pokemon Model", "Model"...
  static const c8* GetKindName(const H3dModel* model);

  /// Reads the position of a model in the world.
  static void GetPosition(const H3dModel* model, f32 out[3]);

  static bool IsVisible(const H3dModel* model);
  static void SetVisible(H3dModel* model, bool is_visible);

  /// The parts of a model: the meshes that the game can show or hide.
  static u32 GetPartCount(const H3dModel* model);
  static const c8* GetPartName(const H3dModel* model, u32 index);
  static bool IsPartVisible(const H3dModel* model, u32 index);
  static void SetPartVisible(H3dModel* model, u32 index, bool is_visible);

  static u32 GetMaterialCount(const H3dModel* model);
  static const c8* GetMaterialName(const H3dModel* model, u32 index);
  static MaterialState ReadMaterial(const H3dModel* model, u32 index);
  static void WriteMaterial(H3dModel* model, u32 index,
                            const MaterialState& state);

private:
  static void RemoveHook(uptr scene, H3dModel* model);

  H3dModel* models_[kMaxModels] = {};
  u32 next_slot_ = 0;
};

} // namespace renderer
