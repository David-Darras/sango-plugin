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

#include "pokemon/patch/model_loader.h"
#include "core/constant/archive_id.h"
#include "core/native/process_manager.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/renderer.h"
#include "renderer/native/h3d_resource.h"
#include "renderer/native/h3d_shader_model.h"
#include "renderer/native/pokemon_model_table.h"
#include "renderer/native/scene.h"
#include "ui/log_application.h"

#define POKEMON_FILE_SPECIES_TABLE (0)
#define POKEMON_FILE_COMMON_SHADER (1)

#define POKEMON_FILE_SECTION_COMMON (0)
#define POKEMON_FILE_SECTION_FACE_ANIMATION (1)
#define POKEMON_FILE_SECTION_TEXTURE_NORMAL (2)
#define POKEMON_FILE_SECTION_TEXTURE_SHINY (3)
#define POKEMON_FILE_SECTION_XXX (4)
#define POKEMON_FILE_SECTION_BATTLE_ANIMATION (5)
#define POKEMON_FILE_SECTION_POKEMON_AMIE_ANIMATION (6)
#define POKEMON_FILE_SECTION_OTHER (7)

namespace pokemon {

bool ModelLoader::LoadOverworldModel(LoadedModel* out, ModelId model_id,
                                     const Vec3& position) {
  if (out == nullptr || out->IsLoaded()) return false;

  renderer::H3dResource* shader =
      overworld::ModelManager::GetInstance().GetShaderResource();
  if (shader == nullptr) {
    ui::LogApplication::Print(u"shader unavailable");
    return false;
  }

  out->model_pack = ReadFile(ArchiveId::kOverworldModel,
                             static_cast<u32>(model_id), true);
  out->model_resource = AttachPackEntry(out->model_pack, 0);
  if (out->model_resource == nullptr) return false;

  void* heap_allocator = HeapAllocator();
  void* device_allocator = DeviceAllocator();
  out->model_resource->Setup(heap_allocator, device_allocator, shader);
  out->model_resource->Setup(heap_allocator, device_allocator, nullptr,
                             renderer::H3dResource::kCommonDefaultShader);
  if (!out->model_resource->IsSetup()) {
    ReportUnresolved(out->model_resource);
    return false;
  }

  if (!BuildModel(out, position)) return false;
  out->model->ApplyOverworldNpcDefaults();
  return true;
}

bool ModelLoader::LoadPokemon(LoadedModel* out, SpeciesId species, FormId form,
                              bool is_shiny, const Vec3& position,
                              Gender gender) {
  if (out == nullptr || out->IsLoaded()) return false;

  renderer::PokemonModelTable table(GetPokemonTable());
  if (!table.IsValid()) return false;

  const u32 pack_top = table.GetPackTop(species, form, gender);
  if (pack_top == renderer::PokemonModelTable::kInvalidPack) {
    ui::LogApplication::Print(u"unknown species %d", species);
    return false;
  }

  const u32 texture_slot =
      is_shiny
        ? POKEMON_FILE_SECTION_TEXTURE_SHINY
        : POKEMON_FILE_SECTION_TEXTURE_NORMAL;

  out->model_pack =
      ReadFile(ArchiveId::kPokemonModel,
               pack_top + POKEMON_FILE_SECTION_COMMON, true);
  out->texture_pack =
      ReadFile(ArchiveId::kPokemonModel, pack_top + texture_slot, true);

  out->model_resource = AttachPackEntry(out->model_pack, 0);
  out->texture_resource = AttachPackEntry(out->texture_pack, 0);
  if (out->model_resource == nullptr) return false;

  void* heap_allocator = HeapAllocator();
  void* device_allocator = DeviceAllocator();

  if (out->texture_resource != nullptr) {
    out->texture_resource->Setup(heap_allocator, device_allocator);
    out->model_resource->Setup(heap_allocator, device_allocator,
                               out->texture_resource);
  }
  LinkPokemonCommonResources(out->model_resource);
  out->model_resource->Setup(heap_allocator, device_allocator, nullptr,
                             renderer::H3dResource::kCommonDefaultShader);
  if (!out->model_resource->IsSetup()) {
    ReportUnresolved(out->model_resource);
    return false;
  }

  return BuildModel(out, position);
}

void ModelLoader::DropAll() {
  void* scene = Scene();
  if (scene == nullptr) return;
  auto& context = GetInstance();
  for (u32 i = 0; i < kMaxLoaded; ++i) {
    LoadedModel* entry = context.loaded_[i];
    if (entry == nullptr) continue;
    if (entry->model != nullptr && scene != nullptr) {
      renderer::Scene::Unregister(scene, entry->model);
      entry->model->Destroy();
      entry->model_resource->RemoveData();
      entry->texture_resource->RemoveData();
    }
    *entry = LoadedModel{};
    context.loaded_[i] = nullptr;
  }
  context.loaded_count_ = 0;
  context.pokemon_table_ = nullptr;
  context.pokemon_common_count_ = 0;
}

void* ModelLoader::Heap() {
  return overworld::MapManager::GetInstance().GetHeap();
}

void* ModelLoader::DeviceHeap() {
  return overworld::MapManager::GetInstance().GetDeviceHeap();
}

void* ModelLoader::HeapAllocator() {
  return overworld::MapManager::GetInstance().GetHeapAllocator();
}

void* ModelLoader::DeviceAllocator() {
  return overworld::MapManager::GetInstance().GetDeviceAllocator();
}

void* ModelLoader::Scene() {
  if (core::ProcessManager::GetInstance().IsCurrentProcess(
      overworld::address::kVtable)) {
    auto* renderer = overworld::Renderer::GetInstance();
    if (renderer == nullptr) return nullptr;
    return renderer->GetScene();
  }
  return nullptr;
}

void* ModelLoader::OpenArchive(ArchiveId archive_id) {
  static void* archives[256] = {};
  const u32 index = static_cast<u32>(archive_id);
  if (index >= 256) return nullptr;
  if (archives[index] != nullptr) return archives[index];

  void* heap = DeviceHeap();
  if (heap == nullptr) return nullptr;

  constexpr u32 kSizeOfArcFile = 120;
  constexpr u32 kOpen = 1 << 0;
  void* archive = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
      heap, kSizeOfArcFile, 4);
  if (archive == nullptr) return nullptr;
  ((void (*)(void*, void*, u32, u32))sys::address::kArchiveInitialize)(
      archive, heap, index, kOpen);
  archives[index] = archive;
  return archive;
}

void* ModelLoader::ReadFile(ArchiveId archive_id, u32 file_id, bool compressed,
                            u32* out_size) {
  void* archive = OpenArchive(archive_id);
  if (archive == nullptr) return nullptr;

  void* device_heap = DeviceHeap();
  u32 size = 0;
  void* buffer = nullptr;

  if (compressed) {
    buffer = ((void* (*)(void*, u32, void*, void*, s32, u32*))
      sys::address::kArchiveLoadCompressedFile)(archive, file_id, Heap(),
                                            device_heap, 128, &size);
  } else {
    size = ((u32 (*)(void*, u32))sys::address::kArchiveGetFileSize)(archive,
      file_id);
    if (size == 0) return nullptr;
    buffer = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
        device_heap, size, 128);
    if (buffer == nullptr) return nullptr;
    ((void (*)(void*, u32, u32, u32, void*))sys::address::kArchiveLoadFile)(
        archive, file_id, 0, size, buffer);
  }

  if (out_size != nullptr) *out_size = size;
  return buffer;
}

renderer::H3dResource* ModelLoader::AttachPackEntry(void* pack, u32 index) {
  if (pack == nullptr) return nullptr;
  auto* bundle = (Bundle*)pack;
  if (index >= bundle->resource_count) return nullptr;

  auto* resource = renderer::H3dResource::Create(DeviceHeap());
  if (resource == nullptr) return nullptr;
  resource->SetData((void*)bundle->GetResource(index));
  return resource;
}

void* ModelLoader::GetPokemonTable() {
  auto& context = GetInstance();
  if (context.pokemon_table_ == nullptr) {
    context.pokemon_table_ =
        ReadFile(ArchiveId::kPokemonModel, POKEMON_FILE_SPECIES_TABLE, false);
  }
  return context.pokemon_table_;
}

void ModelLoader::LinkPokemonCommonResources(renderer::H3dResource* target) {
  auto& context = GetInstance();
  void* heap_allocator = HeapAllocator();
  void* device_allocator = DeviceAllocator();

  if (context.pokemon_common_count_ == 0) {
    void* pack =
        ReadFile(ArchiveId::kPokemonModel, POKEMON_FILE_COMMON_SHADER, false);
    if (pack == nullptr) return;
    auto* bundle = (Bundle*)pack;
    const u32 count = bundle->resource_count;
    for (u32 i = 0; i < count; ++i) {
      if (context.pokemon_common_count_ >= 8) break;
      auto* entry = (u32*)bundle->GetResource(i);
      if ((entry[0] & 0xFFFFFF) != 0x484342) continue;
      auto* resource = renderer::H3dResource::Create(DeviceHeap());
      if (resource == nullptr) break;
      resource->SetData(entry);
      resource->Setup(heap_allocator, device_allocator);
      context.pokemon_common_[context.pokemon_common_count_++] = resource;
    }
  }

  for (u32 i = 0; i < context.pokemon_common_count_; ++i) {
    target->Setup(heap_allocator, device_allocator,
                  context.pokemon_common_[i]);
    if (target->IsSetup()) break;
  }
}

bool ModelLoader::BuildModel(LoadedModel* out, const Vec3& position) {
  out->model = renderer::H3dShaderModel::Create(DeviceHeap());
  if (out->model == nullptr) {
    ui::LogApplication::Print(u"model constructor failed");
    return false;
  }

  renderer::H3dShaderModel::Metadata description =
      renderer::H3dShaderModel::DefaultMetadata();
  description.anim_count = 0;
  out->model->Build(HeapAllocator(), DeviceAllocator(), out->model_resource,
                    0, &description);

  if (out->model->GetMaterialCount() <= 0) {
    ui::LogApplication::Print(u"model has no material");
    return false;
  }

  void* scene = Scene();
  if (scene == nullptr || !renderer::Scene::Register0(scene, out->model) ||
      !renderer::Scene::Register1(scene, out->model)) {
    ui::LogApplication::Print(u"scene registration refused");
    return false;
  }

  out->model->SetTranslate(position);
  Track(out);
  return true;
}

void ModelLoader::Track(LoadedModel* entry) {
  auto& context = GetInstance();
  if (context.loaded_count_ >= kMaxLoaded) return;
  context.loaded_[context.loaded_count_++] = entry;
}

void ModelLoader::ReportUnresolved(renderer::H3dResource* resource) {
  ui::LogApplication::Print(u"setup failed, unresolved %X",
                            resource->GetState());
}

} // namespace pokemon
