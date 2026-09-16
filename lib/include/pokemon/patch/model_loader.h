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
#include "core/constant/archive_id.h"
#include "overworld/constant/model.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/species.h"

namespace renderer {
struct H3dResource;
struct H3dShaderModel;
}

namespace pokemon {

struct LoadedModel {
  void* model_pack = nullptr;
  void* texture_pack = nullptr;
  renderer::H3dResource* model_resource = nullptr;
  renderer::H3dResource* texture_resource = nullptr;
  renderer::H3dShaderModel* model = nullptr;

  INLINE bool IsLoaded() const { return model != nullptr; }
};

class ModelLoader {
  MAKE_SINGLETON(ModelLoader)

public:
  static constexpr u32 kMaxLoaded = 16;

  static bool LoadOverworldModel(LoadedModel* out, ModelId model_id,
                                 const Vec3& position);
  static bool LoadPokemon(LoadedModel* out, SpeciesId species, FormId form,
                          bool is_shiny, const Vec3& position,
                          Gender gender = Gender::kMale);
  static void DropAll();

private:
  static void* Heap();
  static void* DeviceHeap();
  static void* HeapAllocator();
  static void* DeviceAllocator();
  static void* Scene();
  static void* OpenArchive(ArchiveId archive_id);
  static void* ReadFile(ArchiveId archive_id, u32 file_id, bool compressed,
                        u32* out_size = nullptr);
  static renderer::H3dResource* AttachPackEntry(void* pack, u32 index);
  static void* GetPokemonTable();
  static void LinkPokemonCommonResources(renderer::H3dResource* target);
  static bool BuildModel(LoadedModel* out, const Vec3& position);
  static void Track(LoadedModel* entry);
  static void ReportUnresolved(renderer::H3dResource* resource);

  LoadedModel* loaded_[kMaxLoaded] = {};
  u32 loaded_count_ = 0;
  void* pokemon_table_ = nullptr;
  renderer::H3dResource* pokemon_common_[8] = {};
  u32 pokemon_common_count_ = 0;
};

} // namespace pokemon
