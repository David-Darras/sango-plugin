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
 * @file page_model_viewer.cc
 * @brief The menu pages of the 3D models of the screen: the list of the
 *        models, the parts and the materials of one model.
 */

#include <cstdio>

#include "renderer/patch/model_viewer.h"
#include "renderer/patch/skeleton_editor.h"
#include "renderer/patch/texture_editor.h"
#include "ui/paint_application.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using renderer::H3dModel;
using renderer::MaterialState;
using renderer::ModelViewer;

// The menu has 64 entries at most: the lists show the first parts and
// materials.
constexpr u32 kMaxParts = 22;
constexpr u32 kMaxMaterials = 22;
constexpr u32 kLabelLength = 72;

const c8* kCullingNames[] = {"No Culling", "Culling 1", "Culling 2"};

c8 model_labels[ModelViewer::kMaxModels][kLabelLength];

// The model of the pages, and the values that the entries change.
H3dModel* selected_model = nullptr;
bool is_model_visible = true;
f32 model_position[3];
u32 part_count = 0;
bool part_visible[kMaxParts];
c8 part_labels[kMaxParts][kLabelLength];
c8 material_labels[kMaxMaterials][kLabelLength];

// The material of the page of a material.
const void* opened_material = nullptr;
u32 selected_material = 0;
MaterialState material;
bool is_material_loaded = false;
c8 texture_labels[3][kLabelLength];
renderer::UvTransform uv[3];
bool has_uv[3];
c8 uv_labels[3][5][kLabelLength];

bool IsSelectedAlive() { return ModelViewer::IsKnown(selected_model); }

// A change on the page of a model: the model gets the values now.
void ApplyModel(void*) {
  if (!IsSelectedAlive()) return;
  if (ModelViewer::IsVisible(selected_model) != is_model_visible) {
    ModelViewer::SetVisible(selected_model, is_model_visible);
  }
  for (u32 i = 0; i < part_count; i++) {
    if (ModelViewer::IsPartVisible(selected_model, i) != part_visible[i]) {
      ModelViewer::SetPartVisible(selected_model, i, part_visible[i]);
    }
  }
}

void ShowAllParts(void*) {
  for (u32 i = 0; i < part_count; i++) part_visible[i] = true;
  ApplyModel(nullptr);
  MainApplication::GetInstance().Refresh();
}

void HideAllParts(void*) {
  for (u32 i = 0; i < part_count; i++) part_visible[i] = false;
  ApplyModel(nullptr);
  MainApplication::GetInstance().Refresh();
}

// A change on the page of a material.
void ApplyMaterial(void*) {
  if (!IsSelectedAlive()) return;
  ModelViewer::WriteMaterial(selected_model, selected_material, material);
  for (u32 unit = 0; unit < 3; unit++) {
    if (has_uv[unit]) {
      ModelViewer::WriteUv(selected_model, selected_material, unit, uv[unit]);
    }
  }
}
} // namespace

void LoadModelViewerPage(MainApplication& app, void* args) {
#ifdef GAME_ORAS
  H3dModel* models[ModelViewer::kMaxModels];
  const u32 count = ModelViewer::GetModels(models, ModelViewer::kMaxModels);

  app.AddSection("Models On The Screen");
  if (count == 0) {
    app.Add("No 3D model yet.")
       .WithDescription("The viewer knows a model when the game adds it to a "
                        "scene. Open a battle or a menu with 3D models, or "
                        "walk to a new map, then look here again.");
  }
  for (u32 i = 0; i < count; i++) {
    // The position tells the models apart: the Pokemon of the player and the
    // Pokemon of the opponent are not at the same place.
    f32 position[3];
    ModelViewer::GetPosition(models[i], position);
    snprintf(model_labels[i], kLabelLength, "%s %lu  (%lu parts)  %.0f %.0f %.0f",
             ModelViewer::GetKindName(models[i]), (unsigned long)(i + 1),
             (unsigned long)ModelViewer::GetPartCount(models[i]),
             (double)position[0], (double)position[1], (double)position[2]);
    app.Add(model_labels[i], LoadModelPage, models[i])
       .WithDescription("The parts and the materials of this model. The "
                        "numbers are its position. Turn Visible off on its "
                        "page to see which model it is.");
  }
  opened_material = nullptr;
#else
  app.Add("The model viewer needs the data of Alpha Sapphire.");
#endif
}

void LoadModelPage(MainApplication& app, void* args) {
  if (args != nullptr) selected_model = static_cast<H3dModel*>(args);
  if (!IsSelectedAlive()) {
    app.Add("This model is not on the screen any more.")
       .WithDescription("The game destroyed it. Go back and open the list "
                        "again.");
    return;
  }
  H3dModel* model = selected_model;
  opened_material = nullptr;
  is_material_loaded = false;

  is_model_visible = ModelViewer::IsVisible(model);
  ModelViewer::GetPosition(model, model_position);
  part_count = ModelViewer::GetPartCount(model);
  if (part_count > kMaxParts) part_count = kMaxParts;
  for (u32 i = 0; i < part_count; i++) {
    part_visible[i] = ModelViewer::IsPartVisible(model, i);
  }

  app.OnChange(ApplyModel);
  app.AddSection(ModelViewer::GetKindName(model))
     .Add("Visible", is_model_visible)
     .WithDescription("Off: the game does not draw this model.")
     .Add("Position X", model_position[0])
     .WithReadOnly()
     .Add("Position Y", model_position[1])
     .WithReadOnly()
     .Add("Position Z", model_position[2])
     .WithReadOnly();

  if (part_count != 0) {
    app.AddSection("Parts")
       .Add("Show All Parts", ShowAllParts)
       .Add("Hide All Parts", HideAllParts);
    for (u32 i = 0; i < part_count; i++) {
      const c8* name = ModelViewer::GetPartName(model, i);
      snprintf(part_labels[i], kLabelLength, "%s", name[0] != '\0' ? name : "?");
      app.Add(part_labels[i], part_visible[i])
         .WithDescription("A part of the model: a mesh that the game can "
                          "show or hide.");
    }
  }

  if (renderer::SkeletonEditor::GetBoneCount(model) != 0) {
    app.AddSection("Skeleton")
       .Add("Bones", LoadSkeletonPage, model)
       .WithDescription("Moves, turns and scales the bones of the model, "
                        "for example to make the head bigger.");
  }

  u32 material_count = ModelViewer::GetMaterialCount(model);
  if (material_count > kMaxMaterials) material_count = kMaxMaterials;
  if (material_count != 0) app.AddSection("Materials");
  for (u32 i = 0; i < material_count; i++) {
    const c8* name = ModelViewer::GetMaterialName(model, i);
    if (name[0] != '\0') {
      snprintf(material_labels[i], kLabelLength, "%s", name);
    } else {
      snprintf(material_labels[i], kLabelLength, "Material %lu",
               (unsigned long)i);
    }
    app.Add(material_labels[i], LoadMaterialPage,
            reinterpret_cast<void*>(i + 1))
       .WithDescription("The colors, the culling and the alpha test of this "
                        "material.");
  }
}

void LoadMaterialPage(MainApplication& app, void* args) {
  if (!IsSelectedAlive()) {
    app.Add("This model is not on the screen any more.");
    return;
  }
  // A new visit reads the material. After a page of a color, the page keeps
  // its values and writes them to the model.
  if (args != nullptr && args != opened_material) {
    opened_material = args;
    selected_material = reinterpret_cast<uptr>(args) - 1;
    is_material_loaded = false;
  }
  if (!is_material_loaded) {
    material = ModelViewer::ReadMaterial(selected_model, selected_material);
    for (u32 unit = 0; unit < 3; unit++) {
      renderer::TextureUnitInfo info;
      has_uv[unit] = ModelViewer::ReadTextureUnit(selected_model,
                                                  selected_material, unit,
                                                  info) &&
                     info.kind != 0;
      if (has_uv[unit]) {
        uv[unit] = ModelViewer::ReadUv(selected_model, selected_material, unit);
      }
    }
    is_material_loaded = true;
  } else {
    ApplyMaterial(nullptr);
  }

  app.OnChange(ApplyMaterial);
  app.AddSection(material_labels[selected_material < kMaxMaterials
                                     ? selected_material
                                     : 0])
     .Add("Ambient Color", LoadColorPage, &material.ambient)
     .WithDescription("The color of the surface in the shade.")
     .Add("Diffuse Color", LoadColorPage, &material.diffuse)
     .WithDescription("The color of the surface in the light.")
     .Add("Culling", material.culling)
     .WithArray(kCullingNames, SIZE(kCullingNames))
     .WithDescription("Which side of a triangle the GPU hides. Try 1 and 2: "
                      "one of them shows the inside of a model.")
     .Add("Alpha Test", material.has_alpha_test)
     .WithDescription("On: the GPU drops the pixels that are almost "
                      "transparent.");

  // The texture coordinates of each unit that has a texture: scale, turn and
  // move the texture on the surface (for example, to move the eyes).
  for (u32 unit = 0; unit < 3; unit++) {
    if (!has_uv[unit]) continue;
    snprintf(uv_labels[unit][0], kLabelLength, "Texture %lu UV",
             (unsigned long)unit);
    app.AddSection(uv_labels[unit][0])
       .Add("Scale U", uv[unit].scale[0])
       .WithFactor(0.05f)
       .Add("Scale V", uv[unit].scale[1])
       .WithFactor(0.05f)
       .WithDescription("1 is the size of the texture. A bigger number makes "
                        "the texture repeat and smaller.")
       .Add("Rotate", uv[unit].rotate)
       .WithFactor(5.0f)
       .WithDescription("In degrees.")
       .Add("Move U", uv[unit].translate[0])
       .WithFactor(0.01f)
       .Add("Move V", uv[unit].translate[1])
       .WithFactor(0.01f)
       .WithDescription("1 moves the texture by its own size.");
  }

  // The textures that the game makes while it runs (the Pokemon) can be
  // painted.
  app.AddSection("Textures");
  u32 shown = 0;
  for (u32 unit = 0; unit < 3; unit++) {
    renderer::TextureUnitInfo info;
    if (!ModelViewer::ReadTextureUnit(selected_model, selected_material, unit,
                                      info) ||
        info.kind != 3) {
      continue;
    }
    renderer::TextureTarget target;
    const c8* error = nullptr;
    if (renderer::TextureEditor::Locate(selected_model, selected_material, unit,
                                        target, &error)) {
      snprintf(texture_labels[unit], kLabelLength, "Paint Unit %lu  (%s %lux%lu)",
               (unsigned long)unit,
               renderer::TextureEditor::GetFormatName(target.format),
               (unsigned long)target.width, (unsigned long)target.height);
      H3dModel* model = selected_model;
      const u32 material_index = selected_material;
      app.Add(texture_labels[unit],
              [model, material_index, unit](void*) {
                const c8* open_error = nullptr;
                if (!PaintApplication::Open(model, material_index, unit,
                                            &open_error)) {
                  MainApplication::GetInstance().ShowToast(open_error);
                }
              })
         .WithDescription("Draws on this texture with the stylus. The model "
                          "changes at once.");
    } else {
      snprintf(texture_labels[unit], kLabelLength, "Unit %lu  (cannot be painted)",
               (unsigned long)unit);
      app.Add(texture_labels[unit]).WithDescription(error);
    }
    shown++;
  }
  if (shown == 0) app.Add("No texture to paint on this material.");
}

} // namespace ui
