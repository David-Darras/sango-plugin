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
 * @file page_skeleton.cc
 * @brief The menu pages of the skeleton of a 3D model: the list of the bones,
 *        and one bone.
 */

#include <cstdio>

#include "renderer/patch/model_viewer.h"
#include "renderer/patch/skeleton_editor.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using renderer::BoneChange;
using renderer::H3dModel;
using renderer::ModelViewer;
using renderer::SkeletonEditor;

// The menu has 64 entries at most: the list shows a part of the bones.
constexpr u32 kListSize = 24;
constexpr u32 kLabelLength = 48;

H3dModel* selected_model = nullptr;
u32 list_offset = 0;
c8 bone_labels[kListSize][kLabelLength];
c8 changes_note[48];

// The bone of the page of a bone, and the values that its entries change.
u32 selected_bone = 0;
const void* opened_bone = nullptr;
BoneChange change;
f32 uniform_scale = 1.0f;
f32 last_uniform_scale = 1.0f;

bool IsModelAlive() { return ModelViewer::IsKnown(selected_model); }

// A change on the page of a bone: the skeleton gets it at once.
void ApplyChange(void*) {
  if (!IsModelAlive()) return;
  // The entry "Uniform Scale" sets the three scales together.
  if (uniform_scale != last_uniform_scale) {
    last_uniform_scale = uniform_scale;
    change.scale[0] = change.scale[1] = change.scale[2] = uniform_scale;
    MainApplication::GetInstance().Refresh();
  }
  if (change.IsIdentity()) {
    SkeletonEditor::RemoveChange(selected_model, selected_bone);
  } else if (!SkeletonEditor::SetChange(selected_model, selected_bone,
                                        change)) {
    MainApplication::GetInstance().ShowToast(
        "32 changes at most: remove one first.");
  }
}

void ResetBone(void*) {
  if (!IsModelAlive()) return;
  SkeletonEditor::RemoveChange(selected_model, selected_bone);
  change = BoneChange();
  uniform_scale = last_uniform_scale = 1.0f;
  MainApplication::GetInstance().Refresh();
}

void ResetAllBones(void*) {
  if (!IsModelAlive()) return;
  SkeletonEditor::RemoveAll(selected_model);
  MainApplication::GetInstance().Refresh();
}

void SaveChanges(void*) {
  MainApplication::GetInstance().ShowToast(
      SkeletonEditor::Save() ? "Saved to sdmc:/sango/skeleton.ini."
                             : "The SD card did not save the file.");
}

void NextBones(void*) {
  list_offset += kListSize;
  if (list_offset >= SkeletonEditor::GetBoneCount(selected_model)) {
    list_offset = 0;
  }
  MainApplication::GetInstance().Refresh();
}
} // namespace

void LoadSkeletonPage(MainApplication& app, void* args) {
  if (args != nullptr && args != selected_model) {
    selected_model = static_cast<H3dModel*>(args);
    list_offset = 0;
  }
  opened_bone = nullptr;
  if (!IsModelAlive()) {
    app.Add("This model is not on the screen any more.")
       .WithDescription("The game destroyed it. Go back and open the list "
                        "again.");
    return;
  }
#ifdef GAME_ORAS
  const u32 count = SkeletonEditor::GetBoneCount(selected_model);
  if (count == 0) {
    app.Add("This model has no bones.");
    return;
  }

  app.AddSection("Bones");
  for (u32 i = 0; i < kListSize; i++) {
    const u32 bone = list_offset + i;
    if (bone >= count) break;
    const c8* name = SkeletonEditor::GetBoneName(selected_model, bone);
    const bool has_change =
        SkeletonEditor::GetChange(selected_model, bone) != nullptr;
    snprintf(bone_labels[i], kLabelLength, "%s%s", has_change ? "* " : "",
             name[0] != '\0' ? name : "?");
    app.Add(bone_labels[i], LoadBonePage, reinterpret_cast<void*>(bone + 1))
       .WithDescription("Moves, turns and scales this bone and the bones "
                        "under it. A star: this bone has a change.");
  }
  if (count > kListSize) {
    app.Add("More Bones", NextBones)
       .WithDescription("Shows the next bones of the list.");
  }

  snprintf(changes_note, sizeof(changes_note), "%lu changes in all the skeletons",
           (unsigned long)SkeletonEditor::GetChangeCount());
  app.AddSection("Changes")
     .Add(changes_note)
     .Add("Remove The Changes Of This Model", ResetAllBones)
     .WithConfirm()
     .Add("Save To The SD Card", SaveChanges)
     .WithDescription("Writes the changes to sdmc:/sango/skeleton.ini. They "
                      "apply to all the models with the same skeleton, "
                      "after a restart too.");
#else
  app.Add("The skeleton editor needs the data of Alpha Sapphire.");
#endif
}

void LoadBonePage(MainApplication& app, void* args) {
  if (!IsModelAlive()) {
    app.Add("This model is not on the screen any more.");
    return;
  }
  // A new visit reads the change of the bone. After a page of a number the
  // page keeps its values.
  if (args != nullptr && args != opened_bone) {
    opened_bone = args;
    selected_bone = reinterpret_cast<uptr>(args) - 1;
    const BoneChange* saved =
        SkeletonEditor::GetChange(selected_model, selected_bone);
    change = saved != nullptr ? *saved : BoneChange();
    uniform_scale = last_uniform_scale = change.scale[0];
  }

  const c8* name = SkeletonEditor::GetBoneName(selected_model, selected_bone);
  app.OnChange(ApplyChange);
  app.AddSection(name[0] != '\0' ? name : "Bone")
     .Add("Translate X", change.translate[0])
     .WithFactor(1.0f)
     .Add("Translate Y", change.translate[1])
     .WithFactor(1.0f)
     .Add("Translate Z", change.translate[2])
     .WithFactor(1.0f)
     .WithDescription("Moves the bone in its own space, in the units of the "
                      "model.")
     .AddSection("Turn")
     .Add("Rotate X", change.rotate[0])
     .WithFactor(2.0f)
     .Add("Rotate Y", change.rotate[1])
     .WithFactor(2.0f)
     .Add("Rotate Z", change.rotate[2])
     .WithFactor(2.0f)
     .WithDescription("In degrees. The bones under it turn too.")
     .AddSection("Scale")
     .Add("Uniform Scale", uniform_scale)
     .WithFactor(0.05f)
     .WithDescription("Scales the three axes together.")
     .Add("Scale X", change.scale[0])
     .WithFactor(0.05f)
     .Add("Scale Y", change.scale[1])
     .WithFactor(0.05f)
     .Add("Scale Z", change.scale[2])
     .WithFactor(0.05f)
     .AddSection("This Bone")
     .Add("Reset This Bone", ResetBone)
     .WithDescription("Removes the change of this bone.");
}

} // namespace ui
