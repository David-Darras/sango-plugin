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
 * @file model_loader.h
 * @brief Loads 3D models (Pokémon, characters, decorations) and shows them in the overworld.
 */

#pragma once

#include "common.h"
#include "core/constant/archive_id.h"
#include "overworld/constant/model.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/species.h"
#include "pokemon/patch/model_walk.h"

namespace renderer {
struct H3dResource;
struct H3dShaderModel;
}

namespace pokemon {

/// A model that ModelLoader loaded.
struct LoadedModel {
  void* model_pack = nullptr;
  void* texture_pack = nullptr;
  void* motion_pack = nullptr; ///< The animations of the Pokémon (pack section 5).
  void* motion = nullptr; ///< The motion that moves the bones. Null: no animation.
  void* amie_pack = nullptr; ///< The animations of Pokémon-Amie (pack section 6).
  void* amie_motion = nullptr; ///< The motion of these animations. Can be null.
  bool is_amie = false; ///< true: the motion of Pokémon-Amie plays.
  bool is_one_shot = false; ///< true: an animation plays one time, not the idle loop.
  u32 quiet_frames = 0; ///< The frames since the Pokémon moved or played.
  u32 next_action_frames = 0; ///< The Pokémon plays an animation after this number of frames.
  renderer::H3dResource* model_resource = nullptr;
  renderer::H3dResource* texture_resource = nullptr;
  renderer::H3dShaderModel* model = nullptr;
  bool is_resource_shared = false;
  SpeciesId species = SpeciesId::kNone; ///< The species of a Pokémon model.
  FormId form = FormId::kNormal;
  bool is_walking = false; ///< true: the legs of the model swing.
  ModelWalk walk; ///< The legs of the model, and the state of the walk.

  /// Returns true when the model exists.
  INLINE bool IsLoaded() const { return model != nullptr; }
};

/// Loads 3D models and shows them in the overworld. 160 models at most.
class ModelLoader {
  MAKE_SINGLETON(ModelLoader)

public:
  static constexpr u32 kMaxLoaded = 160;

  /// Loads an overworld model at a position.
  static bool LoadOverworldModel(LoadedModel* out, ModelId model_id,
                                 const Vec3& position);
  /// Loads a Pokémon model at a position.
  static bool LoadPokemon(LoadedModel* out, SpeciesId species, FormId form,
                          bool is_shiny, const Vec3& position,
                          Gender gender = Gender::kMale);
  /// Loads a Secret Base decoration at a position.
  static bool LoadDecoration(LoadedModel* out, u32 decoration_index,
                             const Vec3& position);
  /// Reads a model pack from the SD card.
  /// Adds a line to the file sdmc:/sango/follow_trace.txt (the last 24 lines
  /// stay). The file shows the last step before a freeze.
  static void TraceStep(const c8* text, u32 a = 0, u32 b = 0);
  /// Forgets the memory that the loader kept in the heaps of the game: the
  /// table of the species, the shaders of the Pokémon and the archives. Call
  /// it when the game frees these heaps (a new map, a menu that unloads the
  /// overworld). The next load reads them again.
  static void ForgetGameMemory();
  /// Forgets a model without touching the scene or the memory of the game.
  /// Use it when the game destroyed the scene of the model (a battle, a menu,
  /// a new map): the model is not there any more.
  static void Discard(LoadedModel* entry);
  /// Moves the animation of all the Pokémon models forward one frame. Call it
  /// one time for each frame.
  static void UpdateMotions();
  /// Writes the bones of the animation to a model, if the model is one of the
  /// Pokémon models. The renderer calls it when the game starts to draw a
  /// mesh, after the game calculated the bones. ORAS only.
  static void ApplyMotion(const void* model);
  static void* ReadSdPack(const c16* path, u32 capacity);
  static renderer::H3dResource* LoadPackResource(void* pack, u32 index);
  static bool LoadShared(LoadedModel* out, renderer::H3dResource* resource,
                         const Vec3& position);
  static void FreeBuffer(void* buffer);
  /**
   * @brief Reads one file of an archive of the game into the device heap
   *        of the overworld.
   * @param archive_id The archive.
   * @param file_id The file in the archive.
   * @param compressed true for a file with LZ compression: the function
   *        gives the file after decompression.
   * @param out_size Receives the size of the file, or null.
   * @return The file, or null. Release it with FreeBuffer().
   */
  static void* ReadFile(ArchiveId archive_id, u32 file_id, bool compressed,
                        u32* out_size = nullptr);
  /// Removes a model.
  static void Drop(LoadedModel* entry);
  static void Untrack(LoadedModel* entry);
  /// Removes all the models.
  static void DropAll();

private:
  static void* Heap();
  static void* DeviceHeap();
  static void* HeapAllocator();
  static void* DeviceAllocator();
  static void* Scene();
  static void* OpenArchive(ArchiveId archive_id);
  static renderer::H3dResource* AttachPackEntry(void* pack, u32 index);
  static void* GetPokemonTable();
  static void LinkPokemonCommonResources(renderer::H3dResource* target);
  static bool BuildModel(LoadedModel* out, const Vec3& position);
  static void DestroyMotion(LoadedModel* entry);
  /// Gives the memory of the files of a Pokémon model back to the game. Call it
  /// after the model and its resources are destroyed.
  static void ReleasePacks(LoadedModel* entry);
  /// Gives the light of the Pokémon the colours of the light of the overworld
  /// (the time of the day). ORAS only.
  static void UpdatePokemonLight();
  /// Removes the light of the Pokémon from the scene when no Pokémon model
  /// stays in it. ORAS only.
  static void ReleaseLight(void* scene);
  /// Returns the motion that moves the bones now.
  static void* ActiveMotion(const LoadedModel& entry);
  /// Plays an animation of battle or of Pokémon-Amie one time. @a kind: 0 the
  /// battle, 1 Pokémon-Amie, 2 one of the two at random. Returns true when an
  /// animation starts.
  static bool StartAction(LoadedModel* entry, u32 kind);
  /// Plays the idle loop.
  static void BackToIdle(LoadedModel* entry);

public:
  /// Makes a Pokémon model play an animation now and cry. @a use_amie chooses
  /// the animations of Pokémon-Amie, or the animations of the battles. ORAS only.
  static void Act(LoadedModel* entry, bool use_amie);
private:
  /// Makes the idle animation of a Pokémon model. ORAS only.
  static void AttachMotion(LoadedModel* out, u32 pack_top);
  /// Lights a Pokémon model with the light that the game makes for the Pokémon
  /// of the overworld. ORAS only.
  static void ApplyPokemonLight(renderer::H3dShaderModel* model, void* scene);
  static void Track(LoadedModel* entry);
  static void ReportUnresolved(renderer::H3dResource* resource);

  LoadedModel* loaded_[kMaxLoaded] = {};
  u32 loaded_count_ = 0;
  void* pokemon_table_ = nullptr;
  renderer::H3dResource* pokemon_common_[8] = {};
  u32 pokemon_common_count_ = 0;
};

} // namespace pokemon
