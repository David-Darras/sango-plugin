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
#include "overworld/patch/map_character.h"
#include "renderer/address.h"
#include "ui/main_application.h"
#include "ui/page/enum_names.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"
#include "overworld/address.h"

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
Vec3 tile_before; ///< The tiles of the character when the page loaded.
u32 tile_before_slot = 0xFFFFFFFF; ///< The slot of that character.

// The size of a tile in the world, in units.
constexpr f32 kUnitsPerTile = 18.0f;
// The units of the height for one step of Height (see PlayerCheats::Noclip).
constexpr f32 kUnitsPerHeight = 9.0f;

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

// A change of the page: the character moves for the game and on the screen,
// and its model calculates its matrices again.
//
// The tiles are for the game (the walls, the talk): the position of the world
// and the position of the model are for the screen. The three must stay
// together, or the character is in one place and drawn in another.
void ApplyCharacterChanges(void*) {
  if (character_count == 0) return;
  Model& model = GetSelectedModel();

  // The player chose another character: its tiles are the new reference. They
  // are not a move of the character.
  if (character_slots[selected_character] != tile_before_slot) {
    tile_before = model.map_pos.coords;
    tile_before_slot = character_slots[selected_character];
    return;
  }

  const Vec3 now = model.map_pos.coords;
  const f32 dx = now.x - tile_before.x;
  const f32 dy = now.y - tile_before.y;
  const f32 dz = now.z - tile_before.z;
  overworld::DrawModel* draw_model = model.GetDrawModelOrNull();
  if (dx != 0.0f || dy != 0.0f || dz != 0.0f) {
    const f32 wx = dx * kUnitsPerTile;
    const f32 wy = dy * kUnitsPerHeight;
    const f32 wz = dz * kUnitsPerTile;
    model.world_pos.coords.x += wx;
    model.world_pos.coords.y += wy;
    model.world_pos.coords.z += wz;
    model.draw_pos.x += wx;
    model.draw_pos.y += wy;
    model.draw_pos.z += wz;
    if (draw_model != nullptr) {
      draw_model->position.x += wx;
      draw_model->position.y += wy;
      draw_model->position.z += wz;
    }
    // The place where the character comes back to, and the last place.
    model.init_pos.coords.x += dx;
    model.init_pos.coords.y += dy;
    model.init_pos.coords.z += dz;
    model.prev_pos.coords.x += dx;
    model.prev_pos.coords.y += dy;
    model.prev_pos.coords.z += dz;
    tile_before = now;
  }
  if (draw_model != nullptr) draw_model->flags |= overworld::DrawModel::kDirty;
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
     .WithDescription("Select an animation, then press {A} to play it.");

  // The page changes the position and the scale: it moves the character on
  // the screen too (see ApplyCharacterChanges()).
  tile_before = model.map_pos.coords;
  tile_before_slot = character_slots[selected_character];
  app.OnChange(ApplyCharacterChanges);
  app.AddSection("Position")
     .Add("Tile X", model.map_pos.coords.x)
     .Add("Tile Z", model.map_pos.coords.z)
     .Add("Height", model.map_pos.coords.y)
     .WithDescription("The character moves for the game (walls, talk) and on "
                      "the screen.");

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

  // What the game knows about the character: its id, its script, how it
  // moves. The values come from the events of the map: they are for
  // information (the game reads them when the map loads).
  app.AddSection("Behavior")
     .Add("Character ID", model.id)
     .WithReadOnly()
     .WithDescription("The id of the character on the map. 255: the player.")
     .Add("Live Movement Code", model.move_id)
     .WithReadOnly()
     .WithDescription("The movement code that the character uses now.");
  overworld::CharacterPlacement* placement =
      model.id == overworld::ModelManager::kPlayerId
          ? nullptr
          : overworld::MapCharacter::FindPlacement(model.id);
  if (placement != nullptr) {
    app.Add("Script", reinterpret_cast<u16&>(placement->script_id))
       .WithReadOnly()
       .WithDescription("The script that runs when the player talks to the "
                        "character. 3000 and more: a trainer.")
       .Add("Movement Code", placement->movement_id)
       .WithReadOnly()
       .WithDescription("How the character moves: still, walks around, "
                        "turns...")
       .Add("Move Parameter 1", placement->param0)
       .WithReadOnly()
       .Add("Move Parameter 2", placement->param1)
       .WithReadOnly()
       .Add("Move Parameter 3", placement->param2)
       .WithReadOnly()
       .WithDescription("The parameters of the movement: often the distance "
                        "that the character walks.")
       .Add("Interaction", placement->interaction_kind)
       .WithReadOnly()
       .Add("Hidden By Flag", placement->hide_when_flag_set)
       .WithReadOnly()
       .WithDescription("The event flag that hides the character. 0: never.")
       .Add("Start Tile X", placement->position.tile_x)
       .WithReadOnly()
       .Add("Start Tile Z", placement->position.tile_z)
       .WithReadOnly()
       .Add("Sight Width", placement->trigger_width)
       .WithReadOnly()
       .Add("Sight Depth", placement->trigger_depth)
       .WithReadOnly()
       .WithDescription("The size of the area where a trainer sees the "
                        "player.");
  } else if (model.id != overworld::ModelManager::kPlayerId) {
    app.Add("No Placement Found");
  }

#ifndef GAME_XY
  // The appearance of the model: the game reads it when the map loads. All the
  // characters with the same model code share it.
  if (resource_index < ModelManager::kMaxModels) {
    auto& resource = manager.GetResource(resource_index);
    app.AddSection("Appearance")
       .Add("Hitbox Width", resource.width)
       .WithCallback(RefreshMap)
       .WithDescription("The width in tiles. Press {A} to load the map again.")
       .Add("Hitbox Depth", resource.depth)
       .WithCallback(RefreshMap)
       .Add("Shadow", resource.shadow_kind)
       .WithCallback(RefreshMap)
       .Add("Footprints", resource.footprint_kind)
       .WithCallback(RefreshMap)
       .Add("Reflection", resource.reflection_kind)
       .WithCallback(RefreshMap)
       .Add("Outline", resource.outline_kind)
       .WithCallback(RefreshMap);
  }
#endif
}
} // namespace ui
