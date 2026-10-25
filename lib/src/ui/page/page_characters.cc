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
 * @file page_characters.cc
 * @brief The menu page of the characters of the overworld.
 *
 * The player and the people of the map are the same kind of character, so one
 * page edits all of them: the player is a character like the others.
 */

#include <cstdio>
#include <cstring>

#include "core/native/process_manager.h"
#include "overworld/native/model_manager.h"
#include "renderer/address.h"
#include "ui/main_application.h"
#include "ui/page/enum_names.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using overworld::Model;
using overworld::ModelManager;

constexpr u32 kLabelLength = 40;

// The characters of the map, without the empty slots. The list changes when
// the map changes, so the page builds it each time it loads.
u32 character_slots[ModelManager::kMaxModels];
u32 character_count = 0;
c8 character_labels[ModelManager::kMaxModels][kLabelLength];
const c8* character_label_pointers[ModelManager::kMaxModels];

u32 selected_character = 0; ///< The index in character_slots.
bool is_character_chosen = false;
u16 character_animation = 0;

// Returns the slot of the player.
u32 FindPlayerSlot() {
  auto& manager = ModelManager::GetInstance();
  Model* player = &manager.GetPlayer();
  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    if (&manager.GetModel(i) == player) return i;
  }
  return 0;
}

// Returns the entry of the list of the models that describes the model of a
// character, or kMaxModels. Two characters with the same code share it.
u32 FindResource(Model& model) {
  auto& manager = ModelManager::GetInstance();
  const u16 code = model.GetModelResource().code;
  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    if (manager.GetResource(i).code == code) return i;
  }
  return ModelManager::kMaxModels;
}

// Lists the characters of the map by their names. The player comes first.
// Two characters with the same name get a number: "Woman (2)".
void BuildCharacters() {
  auto& manager = ModelManager::GetInstance();
  const u32 player_slot = FindPlayerSlot();
  character_count = 0;
  for (u32 pass = 0; pass < 2; pass++) {
    for (u32 slot = 0; slot < ModelManager::kMaxModels; slot++) {
      Model& model = manager.GetModel(slot);
      if (!model.IsUsed() || (slot == player_slot) != (pass == 0)) continue;
      const u32 id = static_cast<u32>(model.model_id);
      const c8* name = id < SIZE(kModelNames) ? kModelNames[id] : "Character";
      u32 same = 1;
      for (u32 i = 0; i < character_count; i++) {
        if (strncmp(character_labels[i], name, strlen(name)) == 0 &&
            (character_labels[i][strlen(name)] == '\0' ||
             character_labels[i][strlen(name)] == ' ')) {
          same++;
        }
      }
      const u32 index = character_count++;
      character_slots[index] = slot;
      if (same > 1) {
        snprintf(character_labels[index], kLabelLength, "%s (%lu)", name,
                 (unsigned long)same);
      } else {
        snprintf(character_labels[index], kLabelLength, "%s", name);
      }
      character_label_pointers[index] = character_labels[index];
    }
  }
  if (selected_character >= character_count) selected_character = 0;
}

Model& GetSelectedModel() {
  return ModelManager::GetInstance().GetModel(
      character_slots[selected_character]);
}

void PlayCharacterAnimation(void*) {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(
          overworld::address::kVtable) ||
      character_count == 0) {
    return;
  }
  ((void (*)(Model*, u16, u8))renderer::address::kModelPlayAnimation)(
      &GetSelectedModel(), character_animation, 3);
}

void SelectPlayer(void*) {
  selected_character = 0;
  MainApplication::GetInstance().Refresh();
}

#ifdef GAME_XY
// The draw code reads the copy of the model resource of the character, not the
// list. Copy the change into the list too, then load the map again.
void ApplyModel(void*) {
  auto& manager = ModelManager::GetInstance();
  Model& model = GetSelectedModel();
  const u32 resource_index = FindResource(model);
  if (resource_index < ModelManager::kMaxModels) {
    auto& resource = manager.GetResource(resource_index);
    resource.model_id = model.GetModelResource().model_id;
    resource.uses_outfit = model.GetModelResource().uses_outfit;
  }
  RefreshMap(nullptr);
}
#endif
} // namespace

void LoadCharactersPage(MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  auto& manager = ModelManager::GetInstance();
  if (!is_character_chosen) {
    selected_character = 0; // The player.
    is_character_chosen = true;
  }
  BuildCharacters();
  if (character_count == 0) {
    app.Add("There is no character on this map.");
    return;
  }

  app.AddSection("Character")
     .Add("Character", selected_character)
     .WithArray(character_label_pointers, character_count)
     .WithRefresh()
     .WithDescription("The player and the people of the map are the same: "
                      "choose one, then change it below.")
     .Add("Select The Player", SelectPlayer);

  Model& model = GetSelectedModel();
  app.AddSection("Look");
  const u32 resource_index = FindResource(model);
#ifdef GAME_XY
  auto& own_resource = model.GetModelResource();
  app.Add("Model", own_resource.model_id)
     .WithArray(kModelNames, SIZE(kModelNames))
     .WithCallback(ApplyModel)
     .WithDescription("Loads the map again. Every character with the same "
                      "model code changes.")
     .Add("Use Outfit", own_resource.uses_outfit)
     .WithBounds(0, 1)
     .WithCallback(ApplyModel);
  (void)resource_index;
#else
  if (resource_index < ModelManager::kMaxModels) {
    app.Add("Model", manager.GetResource(resource_index).model_id)
       .WithArray(kModelNames, SIZE(kModelNames))
       .WithCallback(RefreshMap)
       .WithDescription("Loads the map again. Every character with the same "
                        "model code changes.");
  }
#endif
  app.Add("Animation", character_animation)
     .WithCallback(PlayCharacterAnimation)
     .WithDescription("Select an animation, then press A to play it.");

  app.AddSection("Position")
     .Add("Tile X", model.map_pos.coords.x)
     .Add("Tile Z", model.map_pos.coords.z)
     .Add("Height", model.map_pos.coords.y);

  overworld::DrawModel* draw_model = model.GetDrawModelOrNull();
  if (draw_model != nullptr) {
    app.AddSection("Scale")
       .Add("Scale X", draw_model->scale.x)
       .WithFactor(0.2f)
       .Add("Scale Y", draw_model->scale.y)
       .WithFactor(0.2f)
       .Add("Scale Z", draw_model->scale.z)
       .WithFactor(0.2f);
  }
}
} // namespace ui
